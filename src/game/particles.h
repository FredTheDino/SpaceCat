Renderer::ParticleSystem rightLaser;
Renderer::ParticleSystem leftLaser;
Renderer::ParticleSystem middleLaser;

Renderer::ParticleSystem hitEnemy;

Renderer::ParticleSystem stars;

void init_laser_particles() {
    rightLaser = Renderer::create_particle_system(5, 1000, V2(0, 0));
    rightLaser.one_size = true;
    rightLaser.alive_time = {0.2, 0.5};
    rightLaser.position_x = {0.105, 0.105};
    rightLaser.position_y = {-0.03, -0.03};
    rightLaser.velocity_dir = {0, 2*PI};
    rightLaser.spawn_size = {0.008, 0.009};
    rightLaser.velocity = {0.2, 0.5};
    rightLaser.spawn_red = {0.756, 0.756};
    rightLaser.spawn_green = {0.811, 0.811};
    rightLaser.spawn_blue = {0.901, 0.901};

    leftLaser = Renderer::create_particle_system(5, 1000, V2(0, 0));
    leftLaser.one_size = true;
    leftLaser.alive_time = {0.2, 0.5};
    leftLaser.position_x = {-0.11, -0.11};
    leftLaser.position_y = {-0.025, -0.025};
    leftLaser.velocity_dir = {0, 2*PI};
    leftLaser.spawn_size = {0.008, 0.009};
    leftLaser.velocity = {0.2, 0.5};
    leftLaser.spawn_red = {0.756, 0.756};
    leftLaser.spawn_green = {0.811, 0.811};
    leftLaser.spawn_blue = {0.901, 0.901};

    middleLaser = Renderer::create_particle_system(5, 1000, V2(0, 0));
    middleLaser.one_size = true;
    middleLaser.alive_time = {0.2, 0.5};
    middleLaser.position_x = {0, 0};
    middleLaser.position_y = {0.18, 0.18};
    middleLaser.velocity_dir = {0, 2*PI};
    middleLaser.spawn_size = {0.01, 0.015};
    middleLaser.velocity = {0.5, 1};
    middleLaser.spawn_red = {0.756, 0.756};
    middleLaser.spawn_green = {0.811, 0.811};
    middleLaser.spawn_blue = {0.901, 0.901};
}

void init_hit_particles() {
    hitEnemy = Renderer::create_particle_system(5, 1000, V2(0, 0));
    hitEnemy.alive_time = {0.3, 0.6};
    hitEnemy.velocity_dir = {0, 2*PI};
    hitEnemy.spawn_size = {0.003, 0.005};
    hitEnemy.die_red = {0.960, 0.960};
    hitEnemy.die_green = {0.894, 0.894};
    hitEnemy.die_blue = {0.529, 0.529};
    hitEnemy.velocity = {0.1, 1};
}

void init_stars() {
    stars = Renderer::create_particle_system(1, 2000, V2(0, 0));
    stars.alive_time = {6, 15};
    stars.position_x = {-2, 2};
    stars.position_y = {-1, 1};
    stars.velocity = {0, 0};
    stars.spawn_size = {0, 0};
    stars.die_size = {0.01, 0.02};
    stars.relative = true;
}
