showBody=true;
showPanel=false;

//panel
if(showPanel){
    difference(){
    cube([70-5-.5,76-5-.5,1]);

    translate([5.25,8.25,0])
    for (row = [1 : 3])
        for (col = [1 : 3]) {
            translate([(col-1)*20, (row-1)*20, -.1])
            cube([14,14,6]);
        }
    }
    for (col = [1 : 2]) {
        translate([(col-1)*20+21, 9.25, -.1])
        cube([3,52,7]);
        
        
        translate([6.25, (col-1)*20+27, -.1])
        rotate([0,0,-90])
        cube([3,52,7]);
    }
}


//The main body
if(showBody){
union(){
    hull(){
    cube([1,76,1]);
    translate([0,0,18])
    cube([1,76,1]);
    translate([18,0,0])
    cube([1,76,1]);
    }
    hull(){
    translate([83.85,0,0])
    cube([1,76,1]);
    translate([83.85,0,19.2])
    cube([1,76,1]);

    translate([55,0,0])
    cube([1,76,1]);
    }



    cube([83.85,76,4.5]);

    translate([27.5,0,-20])
    rotate([0,-35,0])
    union(){
        difference(){
            cube([70,76,48]);
            //main cutout
            translate([9/2,9/2,4.5])
            cube([70-9,76-9,48-4.4]);
            
            //lip for cover
            translate([5/2,5/2,48-5.49])
            cube([70-5,76-5,5.5]);
            
            //usb cutout
            translate([70-25-7.5,-.1,4])
            cube([28,10,12]);
            
            //45 on main box
            translate([0,-.1,-70])
            rotate([0,-55,0])
            cube([80,80,80]);
        }
        //board
        translate([70-5,4.5,4])
        rotate([0,0,90])
        union(){
            baseheight=.5;
            basewidth=65;
            cube([basewidth,28.5,baseheight]);
            translate([0,(28.5-22.5)/2,0])
            cube([basewidth-5.8,22,3+baseheight]);
            translate([basewidth-5.8,(28.5-22.5)/2,0])
            cube([4,22,3+baseheight+1.6]);


            //end board
            translate([basewidth-5.8,(28.5-22.5)/2,0])
            cube([4,18,3+baseheight+1.6]);
            translate([basewidth-5.8+4,0,0])
            difference(){
                cube([1.8,28.5,3+baseheight+5]);
                translate([-.1,(28.5-20)/2,3+baseheight+1.5])
                cube([3,20,1.5]);
            }

            //front board clip
            translate([0,29/2-1,3+baseheight])
            union(){
                cube([2,2.7,3]); 
                translate([0,0,3])   
                rotate([0,45,0])
                cube([1.7,2.7,2]);
            }
        }
    }
}
}