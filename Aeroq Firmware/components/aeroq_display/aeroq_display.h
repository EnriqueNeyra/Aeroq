#pragma once
#include <Arduino.h>

namespace aeroq_display {

// HA button can call this to do a one-off draw on next tick.
void force_redraw();

/**
 * Called every 5s from YAML.
 * This performs a PARTIAL refresh covering the whole canvas
 * (exactly like your loop), never a full refresh.
 */
void tick_and_draw(float co2_ppm,
                   float pm25_ugm3,
                   float voc_index,
                   float temp_c,
                   float rh_pct,
                   float pm25_alert_thresh);

}  // namespace aeroq_display
