#include "nebula/ui/UILayer.h"
#include "nebula/core/Input.h"
#include "nebula/core/KeyCodes.h"
#include "nebula/math/Camera2D.h"

namespace nebula
{

    UIStackLayout::UIStackLayout(UIRect rect, UIAxis axis, float spacing)
        : m_rect(rect), m_axis(axis), m_spacing(spacing), m_cursor(rect.position)
    {
    }

    UIRect UIStackLayout::next(glm::vec2 size)
    {
        UIRect rect{m_cursor, size};
        if (m_axis == UIAxis::Vertical)
            m_cursor.y += size.y + m_spacing;
        else
            m_cursor.x += size.x + m_spacing;
        return rect;
    }

    bool UIRect::contains(const glm::vec2 &point) const
    {
        return point.x >= position.x && point.y >= position.y &&
               point.x <= position.x + size.x &&
               point.y <= position.y + size.y;
    }

    void UILayer::init(std::shared_ptr<FontRenderer> font, int logicalWidth, int logicalHeight)
    {
        m_font = std::move(font);
        setScreenSize(logicalWidth, logicalHeight);
    }

    void UILayer::setFont(std::shared_ptr<FontRenderer> font)
    {
        m_font = std::move(font);
    }

    void UILayer::setScreenSize(int logicalWidth, int logicalHeight)
    {
        m_screenWidth = logicalWidth;
        m_screenHeight = logicalHeight;
    }

    void UILayer::beginFrame(const Window &window)
    {
        m_commands.clear();
        m_mouseLogicalPos = window.windowToLogical(Input::mousePos());
        m_mouseDown = Input::mouseDown(MouseButton::Left);
        m_mousePressed = Input::mousePressed(MouseButton::Left);
    }

    void UILayer::render(SpriteBatch &batch)
    {
        if (!m_visible || !m_font)
            return;

        Camera2D camera(0.0f, (float)m_screenWidth, (float)m_screenHeight, 0.0f);
        batch.begin(camera.viewProjection());

        for (const auto &command : m_commands)
        {
            switch (command.type)
            {
            case CommandType::Panel:
            {
                const auto &rect = command.rect;
                const auto &style = command.panelStyle;
                batch.drawColorQuad(rect.position.x, rect.position.y,
                                    rect.size.x, rect.size.y, style.borderColor);

                float inset = style.borderThickness;
                if (rect.size.x > inset * 2.0f && rect.size.y > inset * 2.0f)
                {
                    batch.drawColorQuad(rect.position.x + inset, rect.position.y + inset,
                                        rect.size.x - inset * 2.0f, rect.size.y - inset * 2.0f,
                                        style.fillColor);
                }
                break;
            }
            case CommandType::Label:
                m_font->drawText(batch, command.text,
                                 command.textPosition.x, command.textPosition.y,
                                 command.labelStyle.color, command.labelStyle.scale);
                break;
            }
        }

        batch.flush();
    }

    void UILayer::label(const std::string &text, const glm::vec2 &position, UILabelStyle style)
    {
        pushLabel(text, position, style);
    }

    void UILayer::panel(const UIRect &rect, UIPanelStyle style)
    {
        pushPanel(rect, style);
    }

    UIButtonState UILayer::button(const std::string &, const std::string &text,
                                  const UIRect &rect, UIButtonStyle style)
    {
        UIButtonState state;
        state.hovered = rect.contains(m_mouseLogicalPos);
        state.pressed = state.hovered && m_mouseDown;
        state.clicked = state.hovered && m_mousePressed;

        UIPanelStyle panelStyle;
        panelStyle.borderColor = style.borderColor;
        panelStyle.borderThickness = style.borderThickness;
        panelStyle.fillColor = state.pressed ? style.activeColor : (state.hovered ? style.hoverColor : style.normalColor);
        pushPanel(rect, panelStyle);

        if (m_font)
        {
            float textWidth = m_font->measureWidth(text, style.textScale);
            float textY = rect.position.y + (rect.size.y - m_font->lineHeight(style.textScale)) * 0.5f + 4.0f;
            glm::vec2 textPos{
                rect.position.x + (rect.size.x - textWidth) * 0.5f,
                textY};
            pushLabel(text, textPos, UILabelStyle{style.textColor, style.textScale});
        }

        return state;
    }

    UIRect UILayer::anchoredRect(glm::vec2 size, UIAnchor anchor, glm::vec2 offset, UIRect parent) const
    {
        if (parent.size.x <= 0.0f || parent.size.y <= 0.0f)
            parent = screenRect();

        glm::vec2 pos = parent.position;
        switch (anchor)
        {
        case UIAnchor::TopLeft:
            break;
        case UIAnchor::TopCenter:
            pos.x += (parent.size.x - size.x) * 0.5f;
            break;
        case UIAnchor::TopRight:
            pos.x += parent.size.x - size.x;
            break;
        case UIAnchor::CenterLeft:
            pos.y += (parent.size.y - size.y) * 0.5f;
            break;
        case UIAnchor::Center:
            pos.x += (parent.size.x - size.x) * 0.5f;
            pos.y += (parent.size.y - size.y) * 0.5f;
            break;
        case UIAnchor::CenterRight:
            pos.x += parent.size.x - size.x;
            pos.y += (parent.size.y - size.y) * 0.5f;
            break;
        case UIAnchor::BottomLeft:
            pos.y += parent.size.y - size.y;
            break;
        case UIAnchor::BottomCenter:
            pos.x += (parent.size.x - size.x) * 0.5f;
            pos.y += parent.size.y - size.y;
            break;
        case UIAnchor::BottomRight:
            pos.x += parent.size.x - size.x;
            pos.y += parent.size.y - size.y;
            break;
        }
        pos += offset;
        return {pos, size};
    }

    UIRect UILayer::insetRect(UIRect rect, UIEdgeInsets insets) const
    {
        rect.position.x += insets.left;
        rect.position.y += insets.top;
        rect.size.x -= insets.left + insets.right;
        rect.size.y -= insets.top + insets.bottom;
        return rect;
    }

    UIStackLayout UILayer::stack(UIRect rect, UIAxis axis, float spacing) const
    {
        return UIStackLayout(rect, axis, spacing);
    }

    UIRect UILayer::screenRect() const
    {
        return {{0.0f, 0.0f}, {(float)m_screenWidth, (float)m_screenHeight}};
    }

    void UILayer::pushPanel(const UIRect &rect, const UIPanelStyle &style)
    {
        DrawCommand command;
        command.type = CommandType::Panel;
        command.rect = rect;
        command.panelStyle = style;
        m_commands.push_back(std::move(command));
    }

    void UILayer::pushLabel(const std::string &text, const glm::vec2 &position, const UILabelStyle &style)
    {
        DrawCommand command;
        command.type = CommandType::Label;
        command.text = text;
        command.textPosition = position;
        command.labelStyle = style;
        m_commands.push_back(std::move(command));
    }

} // namespace nebula
