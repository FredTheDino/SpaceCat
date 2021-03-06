Vec2 target = V2(0, 0);

enum EntityType {
    AGGRO = 0,
    FLOOF = 1,
    GLOOP = 2,
    COG = 3,
    BOSS = 4,
	NR_OF_ENTITIES = 5
};

struct GameEntity : public Logic::Entity {
    Vec2 position;
    float hp;
    float time;
    Physics::Body body;

    virtual bool is_dead() { return hp <= 0; };
};

struct AggroEnemy : public GameEntity {
    void update(float delta) override;

    void draw() override;

    bool charging = false;
    float charge_time = 1;
    float idle_time = 16;
    float time_offset = 2 * PI * random_real();
    float circling_radius = 0.5;
    float speed = 1;
    Vec2& circling_center = target;
    REGISTER_FIELDS(AGGRO_ENEMY, AggroEnemy, circling_radius, speed, speed);
};

void aggro_enemy_init(AggroEnemy& aggro_enemy, Vec2 position);

struct FloofEnemy : public GameEntity {
    void update(float delta) override;

    void (*strategy)(FloofEnemy *self, f32 delta);

    void draw() override;

    float speed = 0.1;
    REGISTER_FIELDS(FLOOF_ENEMY, FloofEnemy, speed);
};

void floof_enemy_init(FloofEnemy& floof_enemy, Vec3 position);
void floof_enemy_init_linear(FloofEnemy& floof_enemy, Vec3 position, Vec2 target);

struct GloopEnemy : public GameEntity {
    void update(float delta) override;
    void draw() override;

    float speed = 0.05;
    Vec2& player_pos = target;
    REGISTER_FIELDS(GLOOP_ENEMY, GloopEnemy, speed);
};

void gloop_enemy_init(GloopEnemy& gloop_enemy, Vec2 position);

struct GloopBullet : public GameEntity {
    void update(float delta) override;

    void draw() override;

    bool is_dead() override;

    float speed = 0.25;
    float rotation = 0;
    Sprites sprite =
        (random_bit() ? Sprites::GLOOP_PEW_1 : Sprites::GLOOP_PEW_2);
    REGISTER_FIELDS(GLOOP_BULLET, GloopBullet, speed);
};

void gloop_bullet_init(GloopBullet& gloop_bullet, GloopEnemy& shooter);

struct Cog : public GameEntity {
    Vec2 center_position;
    Vec2 velocity = V2(0.15, 0);
    f32 size = 0.15;
    f32 rot_speed = 0.2;
    f32 rot_amp = 0.2;
    f32 rotation = 0;
    u32 init_rotations = (u32)(Logic::now()/0.5f);

    void update(f32 delta) override;

    void draw() override;
};

void cog_init(Cog& cog, Vec2 position);

struct Boss : public GameEntity {
    f32 size, x, y;
	f32 next_bullet = 2;
	Physics::Body body_left;
	Physics::Body body_right;

    void update(f32 delta) override;

    void draw() override;
};

void boss_init(Boss& boss);

struct BossBullet : public GameEntity {
    void update(float delta) override;

    void draw() override;

    bool is_dead() override;

    Vec2 velocity = V2(0, -2)*0.3;
    float rotation = 0;
    float size = 0;
    Sprites sprite = 
        (random_bit() ? Sprites::GLOOP_PEW_1 : Sprites::GLOOP_PEW_2);
    REGISTER_FIELDS(GLOOP_BULLET, GloopBullet, speed);
};

void boss_bullet_init(BossBullet& bullet);

struct Wall: public GameEntity {
	Vec2 position, size;

    void update(f32 delta) override;

    void draw() override;
};

void wall_init(Wall& wall, Vec2 pos);

struct Spawner {
    void update(float delta);

    void set_phase(int phase);

    void set_paused(bool paused);

    void clear();

    void spawn_aggro();

    void spawn_floof_phase1();

    void spawn_floof();

    void spawn_gloop();

    void spawn_gloop_bullet(GloopEnemy&);

    void spawn_cog();

    void spawn_cog(Vec2 pos);

    Logic::EntityID spawn_boss();

    void spawn_boss_bullet();

    std::vector<Logic::EntityID> entities;
private:
    float last_spawn[EntityType::NR_OF_ENTITIES];
    int phase = 0;
    bool paused = false;
    float time = 0;
};

Spawner enemy_spawner;
Spawner cog_spawner;
