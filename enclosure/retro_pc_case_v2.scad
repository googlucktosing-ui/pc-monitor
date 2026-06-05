// Retro PC Style - Desktop Monitor Enclosure for ESP32
// License: CC0 - Based on vintage CRT monitor / desktop PC aesthetic
\ = 60;\n// ===== PARAMETERS (edit these to fit your hardware) =====

// Display module (2.8-inch)
DISP_W = 54;      // visible width (mm)
DISP_H = 46;      // visible height  
MOD_W  = 62;      // module pcb width
MOD_H  = 82;      // module pcb height
MOD_T  = 4;       // module thickness

// Mainboard  
PCB_T  = 14;      // board + tallest component

// Enclosure  
WT     = 2.2;     // wall thickness
GAP    = 0.3;     // assembly gap

// Overall box dimensions  
BW     = 90;      // box width (wider for retro bezel)
BH     = 130;     // box height
BD     = 35;      // box depth

// Display window position  
WX = (BW - DISP_W - GAP) / 2;
WY = 12;          // margin from top
WW = DISP_W + GAP;
WH = DISP_H + GAP;

// Floppy drive (decorative)  
FW = 44;          // floppy slot width
FH = 10;          // floppy slot height
FX = (BW - FW) / 2;
FY = WY + WH + 8;  // position below screen

// Power button  
PBTN_D  = 6;      // power button diameter
PBTN_X  = BW / 2;
PBTN_Y  = FY + FH + 14;

// LED indicator  
LED_D  = 2.5;
LED_X  = PBTN_X - 16;
LED_Y  = PBTN_Y;

// Decorative vent slots  
VENT_W  = 1.5;
VENT_G  = 3;

// DHT11 ventilation (on back)  
DHT_W  = 1.2;
DHT_G  = 2.5;

// USB-C cutout (on back)  
USB_W  = 8.5;
USB_H  = 3.5;
USB_Y  = 20;      // position from bottom of cavity

// Reset button (on back)  
BTN_D  = 3.5;

// Screws  
SCR_D  = 2.5;
SCR_OD = 5;

// Stand/base  
BASE_D  = 55;
BASE_H  = 8;
BASE_W  = BW + 6;
BASE_OFF = 3;     // base extends backward

// ===== DERIVED =====  
CW = max(MOD_W, 58) + GAP;  // cavity width
CH = MOD_H + GAP + 4;       // cavity height  
CD = MOD_T + PCB_T + GAP + 2; // cavity depth

// Cavity position (centered behind window)  
CV_X = (BW - CW) / 2;
CV_Y = WY - 4;

// ===== MAIN BODY (front bezel + side walls + stand) =====  
difference() {
    union() {
        // Main rectangular box
        cube([BW, BH, BD]);
        
        // Stand base - extends backward from bottom
        translate([-BASE_OFF, BH - BASE_H, BD - BASE_OFF])
            cube([BASE_W, BASE_H, BASE_D]);
        
        // Gusset for smooth transition
        translate([-BASE_OFF, BH - BASE_H, BD - BASE_OFF])
            cube([BASE_W, 2, 3]);
    }
    
    // Internal cavity  
    translate([CV_X, CV_Y, WT])
        cube([CW, CH, CD + 1]);
    
    // Display window  
    translate([WX, WY, -1])
        cube([WW, WH, WT + 1]);
    
    // ===== FRONT PANEL DETAILS =====
    // Floppy drive recess (decorative)
    translate([FX, FY, -0.5])
        cube([FW, FH, 2]);
    
    // Floppy slot line
    translate([FX + 4, FY + FH/2 - 0.4, -0.5])
        cube([FW - 8, 0.8, 2.5]);
    
    // Power button hole  
    translate([PBTN_X, PBTN_Y, -0.5])
        cylinder(d = PBTN_D, h = 3);
    
    // LED hole  
    translate([LED_X, LED_Y, -0.5])
        cylinder(d = LED_D, h = 3);
    
    // DHT11 vents (bottom front edge)
    for(x = [12 : DHT_G : BW - 12])
        translate([x, BH - 2, -0.5])
            cube([DHT_W, 2.5, 2]);
    
    // Decor vent lines (retro style, below buttons)
    for(y = [PBTN_Y + 8 : VENT_G * 2 : BH - 8])
        translate([BW/4, y, -0.5])
            cube([BW/2, 0.8, 1.2]);
    
    // Weight reduction in stand
    translate([CV_X, BH - BASE_H + 2, BD])
        cube([CW, BASE_H - 3, BASE_D - 10]);
}

// ===== BACK COVER (print separately) =====
translate([BW + 15, 0, 0]) {
    difference() {
        cube([CW, CH, WT]);
        
        // USB-C  
        translate([CW - WT - USB_W, USB_Y, -1])
            cube([USB_W, USB_H, WT + 2]);
        
        // Reset button  
        translate([CW/2, CH - 15, -1])
            cylinder(d = BTN_D, h = WT + 2);
        
        // Back ventilation slots  
        for(y = [8 : VENT_G + 2 : CH - 12])
            translate([CW - WT - 1, y, -1])
                cube([WT + 2, VENT_W, WT + 2]);
        
        // Screw holes  
        for(i = [0, 1], j = [0, 1]) {
            sx = i ? CW - WT : WT;
            sy = j ? CH - WT : WT;
            translate([sx, sy, -1])
                cylinder(d = SCR_D, h = WT + 2);
        }
    }
    
    // Screw bosses  
    for(i = [0, 1], j = [0, 1]) {
        sx = i ? CW - WT : WT;
        sy = j ? CH - WT : WT;
        translate([sx, sy, WT])
            difference() {
                cylinder(d = SCR_OD, h = CD - 2);
                cylinder(d = SCR_D, h = CD + 1);
            }
    }
}
