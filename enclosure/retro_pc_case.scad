// Retro PC Style - ESP32 Monitor Case - CC0 License
$fn = 40;

WT = 2.0; GAP = 0.4;
BW = 80; BH = 120; BD = 30;
DISP_W = 54; DISP_H = 46;

WW = DISP_W + GAP; WH = DISP_H + GAP;
WX = (BW - WW) / 2; WY = 10;

FW = 38; FH = 6;
FX = (BW - FW) / 2;
FY = WY + WH + 8;

PBTN_D = 5; PBTN_X = BW/2; PBTN_Y = FY + FH + 14;
LED_D = 2; LED_X = PBTN_X - 14; LED_Y = PBTN_Y;

VW = 1.5; VG = 3;
USB_W = 8.5; USB_H = 3.5;
USB_X = BW/2 - USB_W/2; USB_Y = 50;
BASE_D = 50; BASE_H = 10; BASE_W = 84;
SCR_D = 2.5; SCR_OD = 5;

CW = 62.4; CH = 86.4; CD = 20.4;
CV_X = 8.8; CV_Y = 6;

// ===== MAIN BODY =====
difference() {
  union() {
    cube([BW, BH, BD]);
    translate([-2, BH - BASE_H, BD])
      cube([BASE_W, BASE_H, BASE_D]);
    translate([-2, BH - BASE_H, BD - 3])
      cube([BASE_W, 2, 3]);
  }
  translate([CV_X, CV_Y, WT]) cube([CW, CH, CD]);
  translate([WX, WY, -1]) cube([WW, WH, WT+1]);
  translate([FX, FY, -0.5]) cube([FW, FH, 1.5]);
  translate([FX+5, FY+FH/2-0.3, -0.5]) cube([FW-10, 0.6, 2.5]);
  translate([PBTN_X, PBTN_Y, -0.5]) cylinder(d=PBTN_D, h=3);
  translate([LED_X, LED_Y, -0.5]) cylinder(d=LED_D, h=3);
  for(x=[12:VG:BW-12]) translate([x, BH-2, -0.5]) cube([VW, 2, 1.5]);
  for(y=[PBTN_Y+8:VG*2:BH-10]) translate([BW/4, y, -0.5]) cube([BW/2, 0.8, 1.2]);
  translate([CV_X, BH-BASE_H+2, BD]) cube([CW, BASE_H-3, BASE_D-10]);
}

// ===== BACK COVER =====
translate([BW+15, 0, 0]) {
  difference() {
    cube([CW, CH, WT]);
    translate([USB_X-CV_X, USB_Y-CV_Y, -1]) cube([USB_W, USB_H, WT+2]);
    for(y=[5:VG+2:CH-10]) translate([CW-WT-1, y, -1]) cube([WT+2, VW, WT+2]);
    for(i=[0,1], j=[0,1]) {
      sx = i ? CW-WT : WT; sy = j ? CH-WT : WT;
      translate([sx, sy, -1]) cylinder(d=SCR_D, h=WT+2);
    }
  }
  for(i=[0,1], j=[0,1]) {
    sx = i ? CW-WT : WT; sy = j ? CH-WT : WT;
    translate([sx, sy, WT]) difference() {
      cylinder(d=SCR_OD, h=CD-2); cylinder(d=SCR_D, h=CD+1);
    }
  }
}
