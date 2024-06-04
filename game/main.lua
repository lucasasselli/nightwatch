import "CoreLibs/graphics"
import "CoreLibs/ui"

local ui <const> = playdate.ui
local gfx <const> = playdate.graphics

local bg_load_img = gfx.image.new("res/images/loading")
local bg_load_fade = 1.0

gfx.setBackgroundColor(gfx.kColorBlack)
ui.crankIndicator.clockwise = true
bg_load_img:draw(0, 0)

function playdate.update()
    if game_run then
        -- Run game
        updateC()
    elseif load_done then
        -- Loading complete show crank prompt
        local change, acceleratedChange = playdate.getCrankChange()
        if math.abs(change) > 0.0 then
            bg_load_fade = bg_load_fade - 0.1
        end

        gfx.clear()
        bg_load_img:drawFaded(0, 0, bg_load_fade, gfx.image.kDitherTypeBayer8x8)

        ui.crankIndicator:update()

        if bg_load_fade <= 0.0 then
            gfx.clear()
            game_run = true
        end
    else
        -- Initialize C
        load_step, load_step_cnt = loadC()

        gfx.setColor(gfx.kColorWhite);
        gfx.fillRect(50, 220, load_step * (300 / load_step_cnt), 3)

        load_done = load_step_cnt == load_step;
    end
end
