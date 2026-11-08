# CAD

Units are mm. One folder per group, each with its own `.blend` (geometry only) and one STL per part, already in print orientation.

| Folder | Part | Qty |
|---|---|---|
| chassis | base_plate | 1 |
| chassis | servo_bracket_L / _R | 1 each |
| chassis | edge_bracket_L / _R | 1 each |
| chassis | caster_skid | 1 |
| enclosure | face_panel, back_panel, lid | 1 each |
| enclosure | side_wall_L / _R | 1 each |
| boards | pico_mount_comms, pico_mount_control | 1 each |
| boards | battery_tray, imu_mount | 1 each |
| face_modules | matrix_frame_L / _R | 1 each |
| face_modules | oled_frame, buzzer_cup | 1 each |
| face_modules | sonar_frame_L / _R | 1 each |

Wheels are reference geometry only (the FS90R kit) and are not exported.

`parts_report.txt` has sizes, volumes and the overlap check.

Fasteners: M3 screws, hex nut traps in the plate; pilot holes (2.5 mm) in the posts for self-tapping M3. Unverified dimensions: OLED and IMU boards, buzzer diameter (12 mm assumed).
