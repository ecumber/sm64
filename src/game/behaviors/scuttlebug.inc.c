// scuttlebug.inc.c

struct ObjectHitbox sScuttlebugHitbox = {
    /* interactType:      */ INTERACT_BOUNCE_TOP,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 1,
    /* health:            */ 1,
    /* numLootCoins:      */ 3,
    /* radius:            */ 130,
    /* height:            */ 70,
    /* hurtboxRadius:     */ 90,
    /* hurtboxHeight:     */ 60,
};

s32 update_angle_from_move_flags(s32 *angle) {
    if (o->oMoveFlags & OBJ_MOVE_HIT_WALL) {
        *angle = o->oWallAngle;
        return 1;
    } else if (o->oMoveFlags & OBJ_MOVE_HIT_EDGE) {
        *angle = o->oMoveAngleYaw + 0x8000;
        return -1;
    }
    return 0;
}

void bhv_scuttlebug_loop(void) {
    UNUSED u8 filler[4];
    f32 sp18;

    cur_obj_update_floor_and_walls();

    if (o->oSubAction != 0
        && cur_obj_set_hitbox_and_die_if_attacked(&sScuttlebugHitbox, SOUND_OBJ_DYING_ENEMY1,
                                                  o->oScuttlebugUnkF4)) {
        o->oSubAction = 3;
    }

    if (o->oSubAction != 1) {
        o->oScuttlebugUnkF8 = 0;
    }

    switch (o->oSubAction) {
        case 0:
            if (o->oMoveFlags & OBJ_MOVE_LANDED) {
                cur_obj_play_sound_2(SOUND_OBJ_GOOMBA_ALERT);
            }
            if (o->oMoveFlags & OBJ_MOVE_MASK_ON_GROUND) {
                o->oHomeX = o->oPosX;
                o->oHomeY = o->oPosY;
                o->oHomeZ = o->oPosZ;
                o->oSubAction++;
            }
            break;

        case 1:
            // oScuttlebugUnkF8 = is lunging (4 bytes)
            // oScuttlebugUnkFC = lunge timer (4 bytes)
            o->oForwardVel = 5.0f;
            if (cur_obj_lateral_dist_from_mario_to_home() > 1000.0f) {
                o->oAngleToMario = cur_obj_angle_to_home();
            } else {
                if (o->oScuttlebugUnkF8 == 0) {
                    o->oScuttlebugUnkFC = 0;
                    o->oAngleToMario = obj_angle_to_object(o, gMarioObject);
                    if (abs_angle_diff(o->oAngleToMario, o->oMoveAngleYaw) < 0x800) {
                        o->oScuttlebugUnkF8 = 1;
                        o->oVelY = 20.0f;
                        cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
                    }
                    // FIXED ! don't lunge when deactivated
                } else if ( (o->oScuttlebugUnkF8 == 1) && (o->activeFlags != ACTIVE_FLAG_DEACTIVATED) ) {
                    o->oForwardVel = 15.0f;
                    o->oScuttlebugUnkFC++;
                    if (o->oScuttlebugUnkFC > 50) {
                        o->oScuttlebugUnkF8 = 0;
                    }
                }
            }
            if (update_angle_from_move_flags(&o->oAngleToMario)) {
                o->oSubAction = 2;
            }
            cur_obj_rotate_yaw_toward(o->oAngleToMario, 0x200);
            break;

        case 2:
            o->oForwardVel = 5.0f;
            if ((s16) o->oMoveAngleYaw == (s16) o->oAngleToMario) {
                o->oSubAction = 1;
            }
            if (o->oPosY - o->oHomeY < -200.0f) {
                obj_mark_for_deletion(o);
            }
            cur_obj_rotate_yaw_toward(o->oAngleToMario, 0x400);
            break;

        case 3:
            o->oFlags &= ~OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW;
            o->oForwardVel = -10.0f;
            o->oVelY = 30.0f;
            cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
            o->oSubAction++;
            break;

        case 4:
            o->oForwardVel = -10.0f;
            if (o->oMoveFlags & OBJ_MOVE_LANDED) {
                o->oSubAction++;
                o->oVelY = 0.0f;
                o->oScuttlebugUnkFC = 0;
                o->oFlags |= OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW;
                o->oInteractStatus = 0;
            }
            break;

        case 5:
            o->oForwardVel = 2.0f;
            o->oScuttlebugUnkFC++;
            if (o->oScuttlebugUnkFC > 30) {
                o->oSubAction = 0;
            }
            break;
    }

    if (o->oForwardVel < 10.0f) {
        sp18 = 1.0f;
    } else {
        sp18 = 3.0f;
    }

    cur_obj_init_animation_with_accel_and_sound(0, sp18);

    if (o->oMoveFlags & OBJ_MOVE_MASK_ON_GROUND) {
        set_obj_anim_with_accel_and_sound(1, 23, SOUND_OBJ2_SCUTTLEBUG_WALK);
    }

    if (o->parentObj != o) {
        if (obj_is_hidden(o)) {
            obj_mark_for_deletion(o);
        }
        if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
            o->parentObj->oScuttlebugSpawnerUnk88 = 1;
        }
    }

    cur_obj_move_standard(-50);
}

void bhv_scuttlebug_spawn_loop(void) {
    if (o->oAction == 0) {
        if (o->oTimer > 30 && 500.0f < o->oDistanceToMario && o->oDistanceToMario < 1500.0f) {
            struct Object *scuttlebug;
            cur_obj_play_sound_2(SOUND_OBJ2_SCUTTLEBUG_ALERT);
            scuttlebug = spawn_object(o, MODEL_SCUTTLEBUG, bhvScuttlebug);
            scuttlebug->oScuttlebugUnkF4 = o->oScuttlebugSpawnerUnkF4;
            scuttlebug->oForwardVel = 30.0f;
            scuttlebug->oVelY = 80.0f;
            o->oAction++;
            o->oScuttlebugUnkF4 = 1;
        }
    } else if (o->oScuttlebugSpawnerUnk88 != 0) {
        o->oScuttlebugSpawnerUnk88 = 0;
        o->oAction = 0;
    }
}
