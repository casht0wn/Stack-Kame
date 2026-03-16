include <BOSL2/std.scad>
include <BOSL2/gears.scad>

$fn = 100;

// this is somewhere between 0.7-0.8
// depends on fidility of the printer so may be try with couple of options
inner_pitch = 0.7;
teeth_count = 20;
screw_hole_diameter = 2;
hub_diameter = 8;
hub_thickness = 4;
arm_length = 20;
arm_end_width = 5;
arm_thickness = 2;


module horn() {
    tag_scope("horn") {
        diff() {
            cyl(d = hub_diameter, h = hub_thickness/2, anchor = BOTTOM) {
                position(TOP) cyl(d = hub_diameter, h = hub_thickness);
                tag("remove") {
                    position(TOP)
                        spur_gear(circ_pitch = inner_pitch, teeth = teeth_count, thickness = hub_thickness, shaft_diam = 0, anchor = BOTTOM);
                    position(CENTER)
                        cyl(d = screw_hole_diameter, h = hub_thickness*2);
                }
            }

            linear_extrude(height = arm_thickness)
                trapezoid(h = arm_length, w1 = arm_end_width, w2 = hub_diameter, anchor = BACK, rounding = [0, 0, arm_end_width/2, arm_end_width/2]);
        }
    }
}

horn();