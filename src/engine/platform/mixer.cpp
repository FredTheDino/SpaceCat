namespace Mixer {


enum VoiceType {
    SINE,
    NOISE,
};

struct Instrument {
    VoiceType voice;
    f32 time;
    f32 pitch;
    f32 pitch_dest;
    f32 pitch_speed;
    f32 gain;
    f32 gain_dest;
    f32 gain_speed;

    f32 time_to;
};

struct SoundSource {
    f32 sample;
    AssetID source;
    u32 track_id;
    f32 pitch;
    f32 gain;
    bool looping;
    bool positional;
    Vec2 position;

    u8 gen;
};

const u32 NUM_INSTRUMENTS = 10;
const u32 NUM_SOURCES = 32;
const u32 NUM_TRACKS = 10;
const u32 TRACK_BUFFER_LENGTH = AUDIO_SAMPLE_RATE * 3 * 2;  // 3 seconds, 2 channels

struct AudioStruct {
    Instrument instruments[NUM_INSTRUMENTS];
    SoundSource sources[NUM_SOURCES];
    u16 num_free_sources;
    u16 free_sources[NUM_SOURCES];
    f32 *tracks[NUM_TRACKS];
    u32 sample_index;
    // Position of the listener
    Vec2 position;
    f32 time;
    f32 time_step;

    SDL_AudioDeviceID dev;
} audio_struct = {};

f32 pitch(s32 tone) {
    return BASE_TONE * pow(NEXT_TONE, tone);
}


void set_note(u32 instrument_id, f32 new_pitch, f32 new_gain, f32 time=1.0) {
    ASSERT(instrument_id < NUM_INSTRUMENTS,
            "Invalid instrument");
    Instrument *inst = audio_struct.instruments + instrument_id;
    inst->pitch_dest = new_pitch;
    inst->pitch_speed = (new_pitch - inst->pitch) / time;
    inst->gain_dest = new_gain;
    inst->gain_speed = (new_gain - inst->gain) / time;
    inst->time_to = time;
}

AudioID push_sound(SoundSource source) {
    lock_audio();
    if (audio_struct.num_free_sources) {
        u16 source_id =
            audio_struct.free_sources[--audio_struct.num_free_sources];
        source.gen = audio_struct.sources[source_id].gen + 1;
        audio_struct.sources[source_id] = source;
        unlock_audio();
        return {source.gen, source_id};
    } else {
        ERR("Not enough free sources, skipping playing of sound");
    }
    unlock_audio();
    return {0, NUM_SOURCES};
}

AudioID play_sound(AssetID asset_id, u32 track_id, f32 pitch, f32 gain, f32 pitch_variance,
                   f32 gain_variance, bool loop) {
    return push_sound({0, asset_id, track_id, pitch + random_real(-1, 1) * pitch_variance,
                       gain + random_real(-1, 1) * gain_variance, loop});
}

AudioID play_sound_at(AssetID asset_id, Vec2 position, u32 track_id, f32 pitch, f32 gain,
                      f32 pitch_variance, f32 gain_variance, bool loop) {
    return push_sound({0, asset_id, track_id, pitch + random_real(-1, 1) * pitch_variance,
                       gain + random_real(-1, 1) * gain_variance, loop, true,
                       position});
}

void stop_sound(AudioID id) {
    ASSERT(id.slot < NUM_SOURCES, "Invalid index in ID");
    lock_audio();
    SoundSource *source = audio_struct.sources + id.slot;
    CHECK(source->gen == id.gen, "Invalid AudioID, the handle is outdated");
    if (source->gen == id.gen) {
        audio_struct.free_sources[audio_struct.num_free_sources++] = id.slot;
        source->gain = 0.0;
    } else {
        ERR("Invalid removal of AudioID that does not exist");
    }
    unlock_audio();
}

void lock_audio() {
    SDL_LockAudioDevice(audio_struct.dev);
}

void unlock_audio() {
    SDL_UnlockAudioDevice(audio_struct.dev);
}

#define S16_TO_F32(S) ((f32) (S) / ((f32) 0xEFFF))

void audio_callback(void* userdata, u8* stream, int len) {
    const u32 SAMPLES = len / sizeof(f32);
    AudioStruct *data = (AudioStruct *) userdata;
    f32 *output_stream = (f32*) stream;
    const f32 TIME_STEP = data->time_step;

    u32 base = audio_struct.sample_index;
    for (u32 track = 0; track < NUM_TRACKS; track++) {
        for (u32 i = 0; i < SAMPLES; i++)
            audio_struct.tracks[track][(base + i) % TRACK_BUFFER_LENGTH] = 0.0;
    }

    f32 left_fade[NUM_SOURCES];
    f32 right_fade[NUM_SOURCES];
    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        SoundSource *source = data->sources + source_id;
        if (source->positional && source->gain != 0) {
            Vec2 distance = source->position - data->position;
            f32 distance_sq = length_squared(distance);
            f32 falloff = 1.0 / MAX(1.0, distance_sq);
            // NOTE(ed): The lengths cancel out, so this is linear
            // falloff.
            f32 left_dot = dot(distance, V2(1, 0));
            left_fade[source_id] = (left_dot + 1.0) / 2.0 * falloff;
            f32 right_dot = dot(distance, V2(-1, 0));
            right_fade[source_id] = (right_dot + 1.0) / 2.0 * falloff;
        }
    }


    for (u32 source_id = 0; source_id < NUM_SOURCES; source_id++) {
        f32 time = 0;
        for (u32 i = 0; i < SAMPLES; i += 2) {
            time += TIME_STEP;
            SoundSource *source = data->sources + source_id;
            if (source->gain == 0.0) break;
            Sound *sound = Asset::fetch_sound(source->source);
            source->sample += sound->sample_rate * source->pitch * TIME_STEP;
            u64 index = source->sample;
            if (index >= sound->num_samples) {
                if (source->looping) {
                    index = 0;
                    source->sample = 0;
                } else {
                    data->free_sources[data->num_free_sources++] = source_id;
                    source->gain = 0.0;
                    break;
                }
            }

            f32 left;
            f32 right;
            if (sound->is_stereo) {
                if (sound->bits_per_sample == 16) {
                    left = S16_TO_F32(sound->samples_16[index * 2 + 0]);
                    right = S16_TO_F32(sound->samples_16[index * 2 + 1]);
                } else if (sound->bits_per_sample == 32) {
                    left = sound->samples_32[index * 2 + 0];
                    right = sound->samples_32[index * 2 + 1];
                } else {
                    UNREACHABLE;
                }
                left *= source->gain;
                right *= source->gain;
            } else {
                f32 sample;
                if (sound->bits_per_sample == 16) {
                    sample = (f32) sound->samples_16[index] / ((f32) 0xEFFF);
                } else if (sound->bits_per_sample == 32) {
                    sample = sound->samples_32[index];
                } else {
                    UNREACHABLE;
                }

                // Distance blending
                left = sample * source->gain;
                right = sample * source->gain;
                if (source->positional) {
                    left *= left_fade[source_id];
                    right *= right_fade[source_id];
                }
            }
            u32 sample_index = (audio_struct.sample_index + i) % TRACK_BUFFER_LENGTH;
            audio_struct.tracks[source->track_id][sample_index+0] += left;
            audio_struct.tracks[source->track_id][sample_index+1] += right;
        }
    }
    for (u32 i = 0; i < SAMPLES; i++)
        output_stream[i] = 0.0;

    for (u32 track_id = 0; track_id < NUM_TRACKS; track_id++) {
        f32 *track = audio_struct.tracks[track_id];
        for (u32 i = 0; i < SAMPLES; i++) {
            output_stream[i] += track[(base + i) % TRACK_BUFFER_LENGTH];
        }
    }
    audio_struct.sample_index += SAMPLES;  // wraps after ~24h
}

bool init() {
    audio_mixer.arena = Util::request_arena();

    audio_struct.num_free_sources = NUM_SOURCES;
    for (u32 i = 0; i < NUM_SOURCES; i++)
        audio_struct.free_sources[i] = i;

    for (u32 i = 0; i < NUM_TRACKS; i++)
        audio_struct.tracks[i] = audio_mixer.arena->push<f32>(TRACK_BUFFER_LENGTH);

    SDL_AudioSpec want = {};
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_F32;
    want.samples = 2048;
    want.channels = 2;
    want.callback = audio_callback;
    audio_struct.time_step = 1.0 / (f32) AUDIO_SAMPLE_RATE;
    want.userdata = (void *) &audio_struct;

    // Let SDL handle the translation for us.
    SDL_AudioSpec have;
    audio_struct.dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_struct.dev <= 0) {
        ERR("%s", SDL_GetError());
        return false;
    }

    SDL_PauseAudioDevice(audio_struct.dev, 0);
    return true;
}

};  // namespace Mixer
