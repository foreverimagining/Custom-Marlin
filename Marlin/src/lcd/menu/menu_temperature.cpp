/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

//
// Temperature Menu
//

#include "../../inc/MarlinConfig.h"

#if HAS_MARLINUI_MENU && HAS_TEMPERATURE

#include "menu_item.h"
#include "../../module/temperature.h"

#if HAS_FAN || ENABLED(SINGLENOZZLE)
  #include "../../module/motion.h"
#endif

#if ANY(HAS_COOLER, LASER_COOLANT_FLOW_METER)
  #include "../../feature/cooler.h"
#endif

#if ENABLED(SINGLENOZZLE_STANDBY_TEMP)
  #include "../../module/tool_change.h"
#endif

//
// "Temperature" submenu items
//

  #if HAS_PREHEAT
    //
    // Preheat for all Materials
    //
    for (uint8_t m = 0; m < PREHEAT_COUNT; ++m) {
      editable.int8 = m;
      #if HAS_MULTI_HOTEND || HAS_HEATED_BED
        SUBMENU_f(ui.get_preheat_label(m), MSG_PREHEAT_M, menu_preheat_m);
      #elif HAS_HOTEND
        ACTION_ITEM_f(ui.get_preheat_label(m), MSG_PREHEAT_M, do_preheat_end_m);
      #endif
    }
  #endif

  #if HAS_TEMP_HOTEND || HAS_HEATED_BED
    //
    // Cooldown
    //
    if (TERN0(HAS_HEATED_BED, thermalManager.degTargetBed())) has_heat = true;
    if (has_heat) ACTION_ITEM(MSG_COOLDOWN, lcd_cooldown);
  #endif

void menu_temperature() {
  #if HAS_TEMP_HOTEND || HAS_HEATED_BED
    bool has_heat = false;
    #if HAS_TEMP_HOTEND
      HOTEND_LOOP() if (thermalManager.degTargetHotend(HOTEND_INDEX)) { has_heat = true; break; }
    #endif
  #endif

  #if HAS_COOLER
    if (thermalManager.temp_cooler.target == 0) thermalManager.temp_cooler.target = COOLER_DEFAULT_TEMP;
  #endif

  START_MENU();
  BACK_ITEM(MSG_MAIN_MENU);

  #if HAS_TEMP_HOTEND || HAS_HEATED_BED
    //
    // Cooldown
    //
    if (TERN0(HAS_HEATED_BED, thermalManager.degTargetBed())) has_heat = true;
    if (has_heat) ACTION_ITEM(MSG_COOLDOWN, lcd_cooldown);
  #endif

  #if HAS_PREHEAT
    //
    // Preheat for all Materials
    //
    LOOP_L_N(m, PREHEAT_COUNT) {
      editable.int8 = m;
      #if HAS_MULTI_HOTEND || HAS_HEATED_BED
        SUBMENU_f(ui.get_preheat_label(m), MSG_PREHEAT_M, menu_preheat_m);
      #elif HAS_HOTEND
        ACTION_ITEM_f(ui.get_preheat_label(m), MSG_PREHEAT_M, do_preheat_end_m);
      #endif
    }
  #endif

  //
  // Nozzle:
  // Nozzle [1-5]:
  //
  #if HOTENDS == 1
    editable.celsius = thermalManager.temp_hotend[0].target;
    EDIT_ITEM_FAST(int3, MSG_NOZZLE, &editable.celsius, 0, thermalManager.hotend_max_target(0), []{ thermalManager.setTargetHotend(editable.celsius, 0); });
  #elif HAS_MULTI_HOTEND
    HOTEND_LOOP() {
      editable.celsius = thermalManager.temp_hotend[e].target;
      EDIT_ITEM_FAST_N(int3, e, MSG_NOZZLE_N, &editable.celsius, 0, thermalManager.hotend_max_target(e), []{ thermalManager.setTargetHotend(editable.celsius, MenuItemBase::itemIndex); });
    }
  #endif

  #if ENABLED(SINGLENOZZLE_STANDBY_TEMP)
    for (uint8_t e = 1; e < EXTRUDERS; ++e)
      EDIT_ITEM_FAST_N(int3, e, MSG_NOZZLE_STANDBY, &thermalManager.singlenozzle_temp[e], 0, thermalManager.hotend_max_target(0));
  #endif

  //
  // Bed:
  //
  #if HAS_HEATED_BED
    EDIT_ITEM_FAST(int3, MSG_BED, &thermalManager.temp_bed.target, 0, BED_MAX_TARGET, thermalManager.start_watching_bed);
  #endif

  //
  // Chamber:
  //
  #if HAS_HEATED_CHAMBER
    EDIT_ITEM_FAST(int3, MSG_CHAMBER, &thermalManager.temp_chamber.target, 0, CHAMBER_MAX_TARGET, thermalManager.start_watching_chamber);
  #endif

  //
  // Cooler:
  //
  #if HAS_COOLER
    bool cstate = cooler.enabled;
    EDIT_ITEM(bool, MSG_COOLER_TOGGLE, &cstate, cooler.toggle);
    EDIT_ITEM_FAST(int3, MSG_COOLER, &thermalManager.temp_cooler.target, COOLER_MIN_TARGET, COOLER_MAX_TARGET, thermalManager.start_watching_cooler);
  #endif

  //
  // Flow Meter Safety Shutdown:
  //
  #if ENABLED(FLOWMETER_SAFETY)
    bool fstate = cooler.flowsafety_enabled;
    EDIT_ITEM(bool, MSG_FLOWMETER_SAFETY, &fstate, cooler.flowsafety_toggle);
  #endif

  //
  // Fan Speed:
  //
  #if HAS_FAN

    DEFINE_SINGLENOZZLE_ITEM();

    #if FAN_IS_M106ABLE(0)
      _FAN_EDIT_ITEMS(0, FIRST_FAN_SPEED);
    #endif
    #if FAN_IS_M106ABLE(1)
      FAN_EDIT_ITEMS(1);
    #elif SNFAN(1)
      singlenozzle_item(1);
    #endif
    #if FAN_IS_M106ABLE(2)
      FAN_EDIT_ITEMS(2);
    #elif SNFAN(2)
      singlenozzle_item(2);
    #endif
    #if FAN_IS_M106ABLE(3)
      FAN_EDIT_ITEMS(3);
    #elif SNFAN(3)
      singlenozzle_item(3);
    #endif
    #if FAN_IS_M106ABLE(4)
      FAN_EDIT_ITEMS(4);
    #elif SNFAN(4)
      singlenozzle_item(4);
    #endif
    #if FAN_IS_M106ABLE(5)
      FAN_EDIT_ITEMS(5);
    #elif SNFAN(5)
      singlenozzle_item(5);
    #endif
    #if FAN_IS_M106ABLE(6)
      FAN_EDIT_ITEMS(6);
    #elif SNFAN(6)
      singlenozzle_item(6);
    #endif
    #if FAN_IS_M106ABLE(7)
      FAN_EDIT_ITEMS(7);
    #elif SNFAN(7)
      singlenozzle_item(7);
    #endif

  #endif // HAS_FAN

  END_MENU();
}

#if ENABLED(PREHEAT_SHORTCUT_MENU_ITEM)

  void menu_preheat_only() {
    START_MENU();
    BACK_ITEM(MSG_MAIN_MENU);

    for (uint8_t m = 0; m < PREHEAT_COUNT; ++m) {
      editable.int8 = m;
      #if HAS_MULTI_HOTEND || HAS_HEATED_BED
        SUBMENU_f(ui.get_preheat_label(m), MSG_PREHEAT_M, menu_preheat_m);
      #else
        ACTION_ITEM_f(ui.get_preheat_label(m), MSG_PREHEAT_M, do_preheat_end_m);
      #endif
    }

    END_MENU();
  }

#endif

#endif // HAS_MARLINUI_MENU && HAS_TEMPERATURE
