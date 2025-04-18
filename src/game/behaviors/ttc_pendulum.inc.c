
/**
 * Behavior for bhvTTCPendulum. This is not the pendulum inside the clock in the
 * castle, but rather the one in TTC itself.
 */

/**
 * Initial angle acceleration.
 */
static f32 sTTCPendulumInitialAccels[] = {
    /* TTC_SPEED_SLOW    */ 13.0f,
    /* TTC_SPEED_FAST    */ 22.0f,
    /* TTC_SPEED_RANDOM  */ 13.0f,
    /* TTC_SPEED_STOPPED */ 0.0f,
};

/**
 * Init function for bhvTTCPendulum.
 */
void bhv_ttc_pendulum_init(void) {
    if (gTTCSpeedSetting != TTC_SPEED_STOPPED) {
        o->oTTCPendulumAngleAccel = sTTCPendulumInitialAccels[gTTCSpeedSetting];
        o->oTTCPendulumAngle = 6500.0f;
    } else {
        o->oTTCPendulumAngle = 6371.5557f;
    }
}

/**
 * Update function for bhvTTCPendulum.
 */
void bhv_ttc_pendulum_update(void) {
    f32 nextChangeInAngleVel;

    #ifdef ECUMBER_DEBUG
    if (gDebugInfo[5][0] != 0)
        o->oTTCPendulumAngleAccel = gDebugInfo[5][0];
    #endif

    if (gTTCSpeedSetting != TTC_SPEED_STOPPED) {
        UNUSED f32 startVel = o->oTTCPendulumAngleVel;

        // Play sound
        if (o->oTTCPendulumSoundTimer != 0) {
            if (--o->oTTCPendulumSoundTimer == 0) {
                cur_obj_play_sound_2(SOUND_GENERAL_PENDULUM_SWING);
            }
        }

        // Stay still for a while
        if (o->oTTCPendulumDelay != 0) {
            o->oTTCPendulumDelay--;
        } else {
            //FIXED ! ttc pendulum speed manipulation
            // this condition shouldn't happen under normal circumstances, so it will only happen if the pendulum was manipulated
            // if the pendulum is within this range (it won't hit angle 0) set it to angle 0 so it will start decelerating
            if ((o->oTTCPendulumAngle <= 12.0f) && (o->oTTCPendulumAngle >= -12.0f))
            {
                o->oTTCPendulumAngle = 0.0f;
            }
            // highest angles the pendulum can reach (that i've checked) rounded to the next multiple of both 13 and 42
            if (o->oTTCPendulumAngle > 7644.0f)
            {
                o->oTTCPendulumAngle = 7644.0f;
            }

            if (o->oTTCPendulumAngle < -7644.0f)
            {
                o->oTTCPendulumAngle = -7644.0f;
            }
            // Accelerate in the direction that moves angle to zero
            if (o->oTTCPendulumAngle * o->oTTCPendulumAccelDir > 0.0f) {
                o->oTTCPendulumAccelDir = -o->oTTCPendulumAccelDir;
            }
            nextChangeInAngleVel = o->oTTCPendulumAngleAccel * o->oTTCPendulumAccelDir;

            #ifdef OLD_PENDULUM_SPEED_FIX
            //1092 is divisible by both 13 and 42 and is above the normal angle velocity
            if ( (o->oTTCPendulumAngleVel + nextChangeInAngleVel) > 1092.0f )
                nextChangeInAngleVel = 1092.0f;
            #endif
            o->oTTCPendulumAngleVel += nextChangeInAngleVel;

            // Ignoring floating point imprecision, angle vel should always be
            // a multiple of angle accel, and so it will eventually reach zero
            // FIXED ! If the pendulum is moving fast enough, the vel could fail to
            //  be a multiple of angle accel, and so the pendulum would continue
            //  oscillating forever
            if (o->oTTCPendulumAngleVel == 0.0f) {
                if (gTTCSpeedSetting == TTC_SPEED_RANDOM) {
                    // Select a new acceleration
                    //! By manipulating this, we can cause the pendulum to reach
                    //  extreme angles and speeds
                    if (random_u16() % 3 != 0) {
                        o->oTTCPendulumAngleAccel = 13.0f;
                    } else {
                        o->oTTCPendulumAngleAccel = 42.0f;
                    }

                    // Pick a random delay
                    if (random_u16() % 2 == 0) {
                        o->oTTCPendulumDelay = random_linear_offset(5, 30);
                    }
                }

                // Play the sound 15 frames after beginning to move
                o->oTTCPendulumSoundTimer = o->oTTCPendulumDelay + 15;
            }

            o->oTTCPendulumAngle += o->oTTCPendulumAngleVel;
        }
    } else {
    }
    // since speed is capped theoretically this should never be too big to cause a crash
    o->oFaceAngleRoll = (s32) o->oTTCPendulumAngle;
    // Note: no platform displacement
}
