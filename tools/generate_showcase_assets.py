#!/usr/bin/env python3
import os
import struct
import zlib


ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def rgba(r, g, b, a=255):
    return (r, g, b, a)


class Image:
    def __init__(self, width, height, fill=(0, 0, 0, 0)):
        self.width = width
        self.height = height
        self.pixels = [fill] * (width * height)

    def set(self, x, y, color):
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def get(self, x, y):
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.pixels[y * self.width + x]
        return (0, 0, 0, 0)

    def rect(self, x, y, w, h, color):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.set(xx, yy, color)

    def outline_rect(self, x, y, w, h, color):
        for xx in range(x, x + w):
            self.set(xx, y, color)
            self.set(xx, y + h - 1, color)
        for yy in range(y, y + h):
            self.set(x, yy, color)
            self.set(x + w - 1, yy, color)

    def blit(self, src, dx, dy):
        for y in range(src.height):
            for x in range(src.width):
                pixel = src.get(x, y)
                if pixel[3] > 0:
                    self.set(dx + x, dy + y, pixel)


def save_png(path, image):
    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data +
                struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    raw = bytearray()
    for y in range(image.height):
        raw.append(0)
        for x in range(image.width):
            raw.extend(image.get(x, y))

    ihdr = struct.pack(">IIBBBBB", image.width, image.height, 8, 6, 0, 0, 0)
    data = b"".join([
        b"\x89PNG\r\n\x1a\n",
        chunk(b"IHDR", ihdr),
        chunk(b"IDAT", zlib.compress(bytes(raw), 9)),
        chunk(b"IEND", b""),
    ])

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


OUTLINE = rgba(18, 24, 40)
SHADOW = rgba(34, 46, 72)
MOSS = rgba(88, 140, 102)
LEAF = rgba(124, 186, 110)
STONE = rgba(70, 88, 112)
STONE_LIGHT = rgba(112, 136, 164)
GOLD = rgba(230, 194, 92)
GOLD_LIGHT = rgba(255, 228, 156)
TEAL = rgba(106, 182, 180)
TEAL_LIGHT = rgba(152, 214, 210)
PLUM = rgba(122, 76, 122)
ROSE = rgba(214, 126, 138)
SKIN = rgba(224, 206, 176)
BOOT = rgba(172, 106, 56)
WISP = rgba(140, 230, 255)
WISP_CORE = rgba(244, 255, 255)
CRYSTAL = rgba(98, 236, 214)
CRYSTAL_LIGHT = rgba(220, 255, 242)
CLOUD = rgba(218, 238, 255, 210)
SPARK = rgba(255, 245, 200)
WOOD = rgba(124, 86, 60)
WOOD_LIGHT = rgba(174, 128, 78)
BLOOM = rgba(255, 130, 142)


def draw_courier_frame(frame_idx, running):
    img = Image(16, 16)
    lantern_x = 11 if frame_idx % 2 == 0 else 12
    hood_shift = 0 if frame_idx in (0, 2) else 1

    # cloak / body
    img.rect(5, 4, 5, 7, OUTLINE)
    img.rect(6, 4, 3, 1, TEAL_LIGHT)
    img.rect(5, 5, 5, 5, TEAL)
    img.rect(6, 6, 3, 3, SHADOW)
    img.rect(6, 5, 2, 2, SKIN)
    img.set(7, 6, rgba(58, 42, 34))
    if frame_idx == 2 and not running:
        img.set(7, 6, OUTLINE)

    # hood silhouette
    img.set(5, 3, OUTLINE)
    img.set(6, 3, TEAL)
    img.set(7, 3, TEAL_LIGHT)
    img.set(8, 3, TEAL)
    img.set(9, 3, OUTLINE)
    img.set(4 + hood_shift, 4, OUTLINE)
    img.set(10 - hood_shift, 4, OUTLINE)

    # scarf / satchel accent
    img.set(8, 8, PLUM)
    img.set(8, 9, PLUM)
    img.set(7, 8, ROSE)

    # legs
    if running:
        leg_patterns = [
            ((6, 10), (8, 10), (5, 12), (9, 12)),
            ((6, 10), (8, 10), (6, 12), (8, 12)),
            ((6, 10), (8, 10), (7, 12), (9, 12)),
            ((6, 10), (8, 10), (6, 12), (8, 12)),
        ]
        for x, y in leg_patterns[frame_idx]:
            img.set(x, y, BOOT)
            if y + 1 < 16:
                img.set(x, y + 1, OUTLINE)
    else:
        img.rect(6, 10, 1, 3, BOOT)
        img.rect(8, 10, 1, 3, BOOT)
        img.set(6, 13, OUTLINE)
        img.set(8, 13, OUTLINE)

    # lantern arm
    img.set(9, 7, WOOD_LIGHT)
    img.set(10, 8, WOOD_LIGHT)
    img.set(10, 9, WOOD_LIGHT)
    img.rect(lantern_x, 8, 2, 3, GOLD)
    img.set(lantern_x, 7, WOOD_LIGHT)
    img.set(lantern_x + 1, 7, OUTLINE)
    img.set(lantern_x, 9, GOLD_LIGHT)
    img.set(lantern_x + 1, 9, GOLD_LIGHT)
    img.set(lantern_x, 11, OUTLINE)
    img.set(lantern_x + 1, 11, OUTLINE)

    return img


def draw_tilesheet():
    sheet = Image(64, 16)

    # Seamless platform block
    tile = Image(16, 16)
    tile.rect(0, 0, 16, 2, LEAF)
    tile.rect(0, 2, 16, 2, MOSS)
    tile.rect(0, 4, 16, 12, STONE)
    tile.rect(0, 5, 16, 1, STONE_LIGHT)
    tile.rect(0, 14, 16, 2, SHADOW)
    for x in range(0, 16, 4):
        tile.set(x + 1, 3, LEAF)
        tile.set(x + 2, 3, MOSS)
    sheet.blit(tile, 0, 0)

    # Stone path
    tile = Image(16, 16)
    tile.rect(0, 0, 16, 16, SHADOW)
    for x, y, w, h in [(1, 1, 6, 5), (8, 1, 7, 5), (2, 7, 5, 4), (8, 7, 6, 4), (1, 12, 6, 3), (8, 12, 7, 3)]:
        tile.rect(x, y, w, h, STONE)
        tile.outline_rect(x, y, w, h, OUTLINE)
        tile.rect(x + 1, y + 1, max(1, w - 2), 1, STONE_LIGHT)
    sheet.blit(tile, 16, 0)

    # Spring tile
    tile = Image(16, 16)
    tile.rect(0, 0, 16, 16, STONE)
    tile.rect(0, 0, 16, 3, LEAF)
    tile.rect(0, 3, 16, 2, MOSS)
    tile.rect(0, 5, 16, 11, STONE)
    tile.rect(2, 6, 12, 6, PLUM)
    tile.rect(3, 7, 10, 4, ROSE)
    tile.rect(4, 5, 2, 1, BLOOM)
    tile.rect(10, 5, 2, 1, BLOOM)
    tile.rect(7, 4, 2, 1, GOLD_LIGHT)
    tile.rect(5, 12, 2, 2, SHADOW)
    tile.rect(9, 12, 2, 2, SHADOW)
    tile.outline_rect(0, 0, 16, 16, OUTLINE)
    sheet.blit(tile, 32, 0)

    # Shrine tile
    tile = Image(16, 16)
    tile.rect(0, 0, 16, 16, STONE)
    tile.rect(2, 2, 12, 12, SHADOW)
    tile.rect(3, 3, 10, 10, STONE_LIGHT)
    tile.rect(4, 4, 8, 8, STONE)
    tile.rect(6, 5, 4, 6, GOLD)
    tile.rect(7, 6, 2, 4, GOLD_LIGHT)
    tile.rect(5, 12, 6, 1, SHADOW)
    tile.outline_rect(0, 0, 16, 16, OUTLINE)
    sheet.blit(tile, 48, 0)

    return sheet


def draw_world_atlas():
    atlas = Image(64, 32)

    # Ember
    sprite = Image(16, 16)
    for x, y in [(7, 2), (6, 3), (8, 3), (5, 4), (9, 4), (6, 5), (8, 5), (7, 6)]:
        sprite.set(x, y, GOLD_LIGHT)
    for x, y in [(7, 1), (5, 3), (9, 3), (4, 4), (10, 4), (5, 5), (9, 5), (6, 6), (8, 6), (7, 7)]:
        sprite.set(x, y, GOLD)
    sprite.rect(6, 7, 3, 4, BLOOM)
    sprite.rect(7, 10, 1, 3, OUTLINE)
    atlas.blit(sprite, 0, 0)

    # Lantern
    sprite = Image(16, 16)
    sprite.rect(6, 2, 4, 1, WOOD_LIGHT)
    sprite.set(6, 3, WOOD_LIGHT)
    sprite.set(9, 3, WOOD_LIGHT)
    sprite.rect(5, 4, 6, 8, WOOD)
    sprite.rect(6, 5, 4, 6, GOLD)
    sprite.rect(7, 6, 2, 4, GOLD_LIGHT)
    sprite.outline_rect(5, 4, 6, 8, OUTLINE)
    atlas.blit(sprite, 16, 0)

    # Wisp
    sprite = Image(16, 16)
    for x, y in [(7, 4), (6, 5), (8, 5), (5, 6), (9, 6), (6, 7), (8, 7), (7, 8)]:
        sprite.set(x, y, WISP_CORE)
    for x, y in [(7, 3), (5, 5), (9, 5), (4, 6), (10, 6), (5, 7), (9, 7), (6, 8), (8, 8), (7, 9)]:
        sprite.set(x, y, WISP)
    sprite.rect(6, 9, 3, 2, rgba(94, 190, 255, 180))
    atlas.blit(sprite, 32, 0)

    # Crystal gate
    sprite = Image(16, 16)
    sprite.rect(7, 2, 2, 10, CRYSTAL)
    sprite.rect(5, 5, 6, 5, CRYSTAL_LIGHT)
    sprite.rect(6, 4, 4, 1, CRYSTAL)
    sprite.rect(6, 10, 4, 1, CRYSTAL)
    sprite.set(4, 7, CRYSTAL)
    sprite.set(11, 7, CRYSTAL)
    for x in range(3, 13):
        sprite.set(x, 13, STONE_LIGHT)
    atlas.blit(sprite, 48, 0)

    # Cloud
    sprite = Image(16, 16)
    for x, y, w, h in [(2, 8, 8, 4), (5, 6, 7, 5), (8, 8, 5, 3)]:
        sprite.rect(x, y, w, h, CLOUD)
    atlas.blit(sprite, 0, 16)

    # Shrub
    sprite = Image(16, 16)
    sprite.rect(5, 9, 6, 5, LEAF)
    sprite.rect(3, 10, 3, 3, MOSS)
    sprite.rect(10, 10, 3, 3, MOSS)
    sprite.rect(7, 6, 2, 4, LEAF)
    sprite.rect(6, 13, 1, 2, WOOD)
    sprite.rect(9, 13, 1, 2, WOOD)
    atlas.blit(sprite, 16, 16)

    # Spark
    sprite = Image(16, 16)
    for x, y in [(7, 3), (7, 4), (7, 5), (7, 6), (5, 5), (6, 5), (8, 5), (9, 5), (6, 4), (8, 4), (6, 6), (8, 6)]:
        sprite.set(x, y, SPARK)
    sprite.set(4, 5, GOLD_LIGHT)
    sprite.set(10, 5, GOLD_LIGHT)
    sprite.set(7, 2, GOLD_LIGHT)
    sprite.set(7, 7, GOLD_LIGHT)
    atlas.blit(sprite, 32, 16)

    # Banner sign
    sprite = Image(16, 16)
    sprite.rect(7, 1, 2, 14, WOOD)
    sprite.rect(4, 3, 8, 6, PLUM)
    sprite.outline_rect(4, 3, 8, 6, OUTLINE)
    sprite.rect(5, 4, 6, 1, GOLD)
    sprite.rect(6, 6, 4, 1, GOLD_LIGHT)
    atlas.blit(sprite, 48, 16)

    return atlas


def draw_particle():
    img = Image(16, 16)
    values = [
        [0, 0, 10, 20, 30, 20, 10, 0],
        [0, 20, 40, 80, 100, 80, 40, 20],
        [10, 40, 90, 140, 180, 140, 90, 40],
        [20, 80, 140, 210, 255, 210, 140, 80],
        [30, 100, 180, 255, 255, 255, 180, 100],
        [20, 80, 140, 210, 255, 210, 140, 80],
        [10, 40, 90, 140, 180, 140, 90, 40],
        [0, 20, 40, 80, 100, 80, 40, 20],
    ]
    for y, row in enumerate(values):
        for x, alpha in enumerate(row):
            img.set(x + 4, y + 4, (255, 240, 200, alpha))
    return img


def main():
    player_sheet = Image(128, 32)
    for idx in range(4):
        player_sheet.blit(draw_courier_frame(idx, running=False), idx * 16, 0)
        player_sheet.blit(draw_courier_frame(idx, running=True), idx * 16, 16)
    save_png(os.path.join(ROOT, "game/assets/generated/player_sheet.png"), player_sheet)

    save_png(os.path.join(ROOT, "game/assets/generated/tiles.png"), draw_tilesheet())
    save_png(os.path.join(ROOT, "game/assets/atlas/world_atlas.png"), draw_world_atlas())
    save_png(os.path.join(ROOT, "game/assets/textures/test.png"), draw_particle())


if __name__ == "__main__":
    main()
