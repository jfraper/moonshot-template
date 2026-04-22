/**
 * @file main.cpp
 * @brief Game built with the Moonshot engine
 *
 * Build and run:
 *   ./scripts/build.sh --run
 */

#include <moonshot.h>

using namespace moonshot;
using namespace moonshot::graphics;

//==============================================================================
// State
//==============================================================================

static Renderer renderer;

//==============================================================================
// game_configure - Called once at startup to set window properties
//==============================================================================

GAME_API void game_configure(GameConfig* config) {
  config->title         = "My Game";
  config->window_width  = 800;
  config->window_height = 600;
}

//==============================================================================
// game_init - Called on startup and after each hot reload
//==============================================================================

GAME_API void game_init(void* saved, u64 size) {
  init_renderer(renderer);
}

//==============================================================================
// game_update - Called every frame
//==============================================================================

GAME_API void game_update(f32 dt) {
  if (input::is_key_pressed(input::keyboard::ESCAPE))
    moonshot::window::close();

  set_background_color(0.09f, 0.09f, 0.14f, 1.0f);

  // Title
  static TextStyle style = {
    .color     = COLORS::WHITE,
    .anchor_x  = TextAnchor::CENTER,
    .font_size = 32.0f,
  };

  draw_text(renderer, Transform::from_translation(400.0f, 250.0f), Text{"Your game starts here.", &style});

  // WASD
  Quad key = {.width = 52.0f, .height = 52.0f, .color = Color4{0.35f, 0.65f, 1.0f, 1.0f}, .border_radius = Corners(8.0f)};
  draw_quad(renderer, Transform::from_translation(400.0f, 300.0f), key);   // W
  draw_quad(renderer, Transform::from_translation(344.0f, 356.0f), key);   // A
  draw_quad(renderer, Transform::from_translation(400.0f, 356.0f), key);   // S
  draw_quad(renderer, Transform::from_translation(456.0f, 356.0f), key);   // D
}
