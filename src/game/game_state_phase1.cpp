namespace Phase1 {

Logic::EntityID player_id;
Physics::Body bordere_rect_container[4];

f32 progess;
f32 saddness;
f32 saddness_target;

Vec4 START_COLOR = V4(0.1, 0.05, 0.05, 1.0);
Vec4 END_COLOR = V4(0.3, 0.3, 0.3, 1.0);

void setup();
void enter();
void update(f32 now, f32 delta);
void draw();
void exit();

Mixer::AudioID tickID;

void setup() {}

Logic::LogicID leave_id;

void enter() {
    current_exit();
    Logic::update_callback(update_id, update, 0.0, Logic::FOREVER);
    Logic::update_callback(draw_id, draw, 0.0, Logic::FOREVER);
    current_exit = exit;

    enemy_spawner.set_phase(1);
    enemy_spawner.set_paused(false);

    cog_spawner.set_phase(11);
    cog_spawner.set_paused(false);

    cog_spawner.spawn_cog(V2(0.6, 0));

    progess = 0;
    saddness_target = 1.0;
    saddness = saddness_target;
    auto leave = []() {
        if (progess >= 1.0) {
            LOG("cut");
            Logic::update_callback(update_id, empty_func, 0.0, Logic::FOREVER);
            Logic::update_callback(draw_id, empty_func, 0.0, Logic::FOREVER);
            transitioning = true;
            Cutscene::enter(1);
        }
    };
    leave_id = Logic::add_callback(Logic::POST_DRAW, leave, 0.0, Logic::FOREVER);

    PlayerPhase1 player;
    player.init();
    player_id = Logic::add_entity(player);
    init_hit_particles();
    tickID = Mixer::play_sound(2, ASSET_METRONOME_2, 1.0, Mixer::AUDIO_DEFAULT_GAIN, Mixer::AUDIO_DEFAULT_VARIANCE, Mixer::AUDIO_DEFAULT_VARIANCE, true);
    transitioning = false;
}

void update(f32 delta, f32 now) {

    if (transitioning) return;

    hitEnemy.update(delta);
    enemy_spawner.update(delta);
    cog_spawner.update(delta);

    PlayerPhase1 *player = (PlayerPhase1 *) Logic::fetch_entity(player_id);
    for (s32 i = cog_spawner.entities.size() - 1; i >= 0; i--) {
        GameEntity *cog =
            Logic::fetch_entity<GameEntity>(cog_spawner.entities[i]);
        if (Physics::check_overlap(&cog->body, &player->body)) {
            cog->hp = 0;
            pick_up_compliment();
            // TODO: Fix this later
            progess = CLAMP(0, 1.0, progess + 0.2);
            saddness_target -= 0.2;
        }
    }

    for (s32 i = enemy_spawner.entities.size() - 1; i >= 0; i--) {
        GameEntity *enemy = Logic::fetch_entity<GameEntity>(enemy_spawner.entities[i]);
        Physics::Overlap overlap = Physics::check_overlap(&enemy->body, &player->body);
        if (overlap) {
            player->body.velocity += overlap.normal * 0.1;
            enemy->body.velocity -= overlap.normal * 0.1;
            hitEnemy.position = (player->body.position + enemy->body.position)/2;
            for (int i = 0; i < 300; i++) {
                hitEnemy.spawn();
            }
            // TODO(ed): Which channel
            AssetID alts[] = {
                ASSET_SPACESUIT_HIT_1,
                ASSET_SPACESUIT_HIT_2,
                ASSET_SPACESUIT_HIT_3,
                ASSET_SPACESUIT_HIT_4,
            };
            Mixer::play_sound(6, alts[random_int() % LEN(alts)]);
            saddness_target += 0.2;
            progess = CLAMP(0, 1.0, progess - 0.2);
        }
    }
    saddness_target = CLAMP(0, 1.0, saddness_target);
    Vec2 target = -player->body.position;
    Vec2 curr = Renderer::get_camera()->position;
    Renderer::get_camera()->position = LERP(curr, 2 * length(target - curr) * delta, target);
}

void draw() {

    if (transitioning) return;

    // Draw background
    Vec4 tint = LERP(START_COLOR, progess, END_COLOR);
    draw_sprite(0, -Renderer::get_camera(0)->position, 2, 0, Sprites::BACKGROUND, tint);

    saddness = LERP(saddness, Logic::delta() * 2, saddness_target);
    Renderer::vignette_strength = (saddness * saddness) * 4.5;
    Renderer::vignette_radius = (saddness * saddness) * 0.5;

    hitEnemy.draw();
    stars.draw();
    // Physics::Overlap curr_overlap =
    // Physics::check_overlap(&player1.player_body, &temp_rect);
    // Physics::solve(curr_overlap);
    // Physics::debug_draw_body(&temp_rect);
}

void exit() {
    Logic::remove_callback(leave_id);
    Logic::remove_entity(player_id);
    enemy_spawner.clear();
    cog_spawner.clear();
    Mixer::stop_sound(tickID);
}
};  // namespace Phase1
