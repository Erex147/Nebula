#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "nebula/core/Window.h"
#include "nebula/renderer/FontRenderer.h"
#include "nebula/renderer/SpriteBatch.h"

namespace nebula
{

    struct UIRect
    {
        glm::vec2 position{0.0f, 0.0f};
        glm::vec2 size{0.0f, 0.0f};

        bool contains(const glm::vec2 &point) const;
    };

    struct UIEdgeInsets
    {
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
    };

    enum class UIAnchor
    {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    enum class UIAxis
    {
        Horizontal,
        Vertical
    };

    class UIStackLayout
    {
    public:
        UIStackLayout(UIRect rect, UIAxis axis, float spacing);

        UIRect next(glm::vec2 size);
        const UIRect &bounds() const { return m_rect; }

    private:
        UIRect m_rect;
        UIAxis m_axis = UIAxis::Vertical;
        float m_spacing = 0.0f;
        glm::vec2 m_cursor{0.0f, 0.0f};
    };

    struct UILabelStyle
    {
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        float scale = 1.0f;
    };

    struct UIPanelStyle
    {
        glm::vec4 fillColor{0.06f, 0.09f, 0.14f, 0.82f};
        glm::vec4 borderColor{1.0f, 1.0f, 1.0f, 0.12f};
        float borderThickness = 2.0f;
    };

    struct UIButtonStyle
    {
        glm::vec4 textColor{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 normalColor{0.12f, 0.16f, 0.24f, 0.86f};
        glm::vec4 hoverColor{0.20f, 0.26f, 0.38f, 0.92f};
        glm::vec4 activeColor{0.30f, 0.40f, 0.56f, 0.95f};
        glm::vec4 borderColor{0.85f, 0.92f, 1.0f, 0.24f};
        float borderThickness = 2.0f;
        float textScale = 1.0f;
    };

    struct UIButtonState
    {
        bool hovered = false;
        bool pressed = false;
        bool clicked = false;
    };

    class UILayer
    {
    public:
        void init(std::shared_ptr<FontRenderer> font, int logicalWidth, int logicalHeight);
        void setFont(std::shared_ptr<FontRenderer> font);
        void setScreenSize(int logicalWidth, int logicalHeight);
        void beginFrame(const Window &window);
        void render(SpriteBatch &batch);

        void label(const std::string &text,
                   const glm::vec2 &position,
                   UILabelStyle style = {});
        void panel(const UIRect &rect,
                   UIPanelStyle style = {});
        UIButtonState button(const std::string &id,
                             const std::string &text,
                             const UIRect &rect,
                             UIButtonStyle style = {});
        UIRect anchoredRect(glm::vec2 size,
                            UIAnchor anchor,
                            glm::vec2 offset = {0.0f, 0.0f},
                            UIRect parent = {}) const;
        UIRect insetRect(UIRect rect, UIEdgeInsets insets) const;
        UIStackLayout stack(UIRect rect, UIAxis axis, float spacing = 0.0f) const;
        UIRect screenRect() const;

        const glm::vec2 &mousePosition() const { return m_mouseLogicalPos; }
        bool visible() const { return m_visible; }
        void setVisible(bool visible) { m_visible = visible; }

    private:
        enum class CommandType
        {
            Panel,
            Label
        };

        struct DrawCommand
        {
            CommandType type;
            UIRect rect;
            UIPanelStyle panelStyle;
            std::string text;
            glm::vec2 textPosition{0.0f, 0.0f};
            UILabelStyle labelStyle;
        };

        std::shared_ptr<FontRenderer> m_font;
        std::vector<DrawCommand> m_commands;
        glm::vec2 m_mouseLogicalPos{0.0f, 0.0f};
        bool m_mouseDown = false;
        bool m_mousePressed = false;
        bool m_visible = true;
        int m_screenWidth = 0;
        int m_screenHeight = 0;

        void pushPanel(const UIRect &rect, const UIPanelStyle &style);
        void pushLabel(const std::string &text, const glm::vec2 &position, const UILabelStyle &style);
    };

} // namespace nebula
