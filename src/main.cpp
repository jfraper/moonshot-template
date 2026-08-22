/**
 * @file main.cpp
 * @brief Moonshot game template.
 *
 * Build and run:
 *   ./scripts/build.sh --run
 */

#include "moonshot/core/str.h"        // str_format
#include "moonshot/graphics/coords.h" // get_visible_rect, screen_to_world
#include "moonshot/ui.h"

#include <moonshot.h>

using namespace moonshot;
using namespace moonshot::graphics;

//==============================================================================================
// COORDINATES
//==============================================================================================
//
//  draw_* and ui:: SHARE ONE SPACE: origin at the CENTRE of the window, +Y UP, design pixels. The
//  same pair of numbers is the same point in both. Only the anchor differs:
//
//    draw_quad(r, Transform::from_translation(0, 120), quad)  // the quad's CENTRE lands there
//    ui::Position::absolute(0, 120, Pivot::center())          // the element's CENTRE lands there
//    ui::Position::absolute(0, 120)                           // its TOP-LEFT lands there instead
//
//  TWO RECTANGLES, and choosing between them has nothing to do with which API you draw with:
//    window::get_size_ref()                  the design box, fixed at the game_configure size
//    get_visible_rect(renderer).size         what the window actually shows, which grows past the
//                                            design box when the window stops matching its aspect
//
//  MOUSE. input::get_mouse_position() is screen space (origin bottom-left, real window points).
//  Convert with screen_to_world(renderer, ...) before hit-testing anything you drew.
//
//  CAMERA. A renderer with no camera draws through an IDENTITY projection and the scene lands
//  nowhere. set_camera() is not optional, and the Camera2D must outlive the renderer.
//
//==============================================================================================

static Renderer         renderer;
static camera::Camera2D cam;

constexpr f32 MARKER = 56.0f;  ///< Side of the corner squares
constexpr f32 ORIGIN = 40.0f;  ///< Side of the square on the origin
constexpr f32 INSET  = 14.0f;  ///< Gap they keep from the window edge
constexpr f32 ORBIT  = 120.0f; ///< Radius of the circle orbiting the origin

static const Color CORAL  = Color(0.93f, 0.35f, 0.42f, 1.0f);
static const Color MINT   = Color(0.36f, 0.82f, 0.52f, 1.0f);
static const Color AMBER  = Color(0.97f, 0.75f, 0.30f, 1.0f);
static const Color VIOLET = Color(0.64f, 0.51f, 0.96f, 1.0f);

// Game state lives here, across frames. game_update advances it, the UI reads and writes it.
static f32  orbit_angle = 0.0f;
static bool orbiting    = true;

GAME_API void game_configure(GameConfig* config) {
  config->title         = "My Game";
  config->window_width  = 900;
  config->window_height = 640;
}

GAME_API void game_init(void* saved, u64 size) {
  init_renderer(renderer);
  set_camera(renderer, cam); // Without this the projection stays identity and nothing lands right
}

/// The ui:: layer: a panel hanging from the top edge and a button standing on the bottom one, both
/// anchored in the very same coordinates the squares use.
static void overlay(vec2f half) {
  const ui::Palette& p = ui::current_palette;
  char               buf[48];

  ui::begin_ui(&renderer, {.layer = 1000});
  ui::begin_container(
    "panel",
    {
      .width            = ui::Size::fit(),
      .height           = ui::Size::fit(),
      .position         = ui::Position::absolute(0.0f, half.y - INSET, Pivot::top_center()),
      .background_color = p.bg_container,
      .border_radius    = 10.0f,
      .padding          = ui::Spacing(16.0f, 24.0f),
      .layout =
        {.align_content_x = ui::Align::CENTER,
         .align_items     = ui::Align::CENTER,
         .direction       = ui::LayoutDirection::TOP_TO_BOTTOM,
         .gap             = 6.0f},
    }
  );

  // Engine fonts are reachable from any game, and shared_font survives a hot reload
  static Font* bold = shared_font(MOONSHOT_DEFAULT_FONT, "bold");
  ui::label("Your game starts here", {.font = bold, .color = p.text_primary, .font_size = 32.0f});

  const vec2f mouse = screen_to_world(renderer, input::get_mouse_position());
  ui::label(
    str_format(buf, sizeof(buf), "mouse in world: (%.0f, %.0f)", mouse.x, mouse.y), {.color = p.text_secondary, .font_size = 12.0f}
  );

  ui::end_container();

  // A second root, anchored to the bottom edge. Widgets report what happened to them this frame,
  // so the click is acted on right where the button is declared.
  ui::begin_container("controls", {.position = ui::Position::absolute(0.0f, -(half.y - INSET), Pivot::bottom_center())});
  if (ui::button("orbit", orbiting ? "Pause" : "Resume").clicked)
    orbiting = !orbiting;
  ui::end_container();

  ui::end_ui();
}

GAME_API void game_update(f32 dt) {
  if (input::is_key_pressed(input::keyboard::ESCAPE))
    moonshot::window::close();

  set_background_color(0.09f, 0.09f, 0.14f, 1.0f);

  // Measured from what the window SHOWS, so the squares hug the real corners at any size. Use
  // window::get_size_ref() * 0.5f instead and they stay pinned to the design box.
  const vec2f half = get_visible_rect(renderer).size * 0.5f;
  const vec2f edge = half - (INSET + MARKER * 0.5f);

  const Color axis = Color(1.0f, 1.0f, 1.0f, 0.10f);
  draw_line(renderer, {-half.x, 0.0f}, {half.x, 0.0f}, axis, 1.0f);
  draw_line(renderer, {0.0f, -half.y}, {0.0f, half.y}, axis, 1.0f);

  draw_quad(renderer, Transform::from_translation(-edge.x, edge.y), {.width = MARKER, .height = MARKER, .color = CORAL});
  draw_quad(renderer, Transform::from_translation(edge.x, edge.y), {.width = MARKER, .height = MARKER, .color = MINT});
  draw_quad(renderer, Transform::from_translation(-edge.x, -edge.y), {.width = MARKER, .height = MARKER, .color = AMBER});
  draw_quad(renderer, Transform::from_translation(edge.x, -edge.y), {.width = MARKER, .height = MARKER, .color = VIOLET});

  // The origin, labelled inside the square it marks
  static TextStyle origin_label = {
    .color     = Color4{0.09f, 0.09f, 0.14f, 1.0f},
    .anchor_x  = TextAnchor::CENTER,
    .anchor_y  = TextAnchorVertical::CENTER,
    .font_size = 12.0f,
  };
  // Text goes through its own shader, so it lands in its own batch: without a higher layer the
  // label would sort behind the square it belongs to.
  draw_quad(renderer, Transform::from_translation(0.0f, 0.0f), {.width = ORIGIN, .height = ORIGIN, .color = COLORS::WHITE});
  draw_text(renderer, Transform::from_translation(0.0f, 0.0f), Text{"0,0", &origin_label}, {.layer = 1});

  // dt is the seconds since the last frame, so multiplying by it keeps the speed independent of
  // the frame rate. The button along the bottom edge is what flips `orbiting`.
  if (orbiting)
    orbit_angle += dt;
  draw_circle(renderer, {math::cos(orbit_angle) * ORBIT, math::sin(orbit_angle) * ORBIT}, 14.0f, MINT);

  overlay(half);
}
