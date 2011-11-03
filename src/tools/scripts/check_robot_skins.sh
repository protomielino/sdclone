
# Script to scan through all robots

# Uncommet for SVN version checking
versioning="-s"
# of for Git-svn version checking
#versioning="-g"

drivers="`pwd`/../../drivers/"
cars="`pwd`/../../../data/cars/models/"

# Enable the screen shots
config="$HOME/.speed-dreams-2"
run="/usr/local/games/speed-dreams-2"

# Check the car models (for humans)
python check_car_skins.py -c $cars $versioning -C $config -r $run

# usr
python check_robot_skins.py -d $drivers/usr/usr_36GP -c $cars $versioning -C $config -r $run usr_36GP.xml
python check_robot_skins.py -d $drivers/usr/usr_ls1  -c $cars $versioning -C $config -r $run usr_ls1.xml
python check_robot_skins.py -d $drivers/usr/usr_ls2  -c $cars $versioning -C $config -r $run usr_ls2.xml
python check_robot_skins.py -d $drivers/usr/usr_sc   -c $cars $versioning -C $config -r $run usr_sc.xml
python check_robot_skins.py -d $drivers/usr/usr_trb1 -c $cars $versioning -C $config -r $run usr_trb1.xml
python check_robot_skins.py -d $drivers/usr/usr_rs   -c $cars $versioning -C $config -r $run usr_rs.xml

# Simplix
python check_robot_skins.py -d $drivers/simplix/simplix_36GP -c $cars $versioning -C $config -r $run simplix_36GP.xml
python check_robot_skins.py -d $drivers/simplix/simplix_ls1  -c $cars $versioning -C $config -r $run simplix_ls1.xml
python check_robot_skins.py -d $drivers/simplix/simplix_ls2  -c $cars $versioning -C $config -r $run simplix_ls2.xml
python check_robot_skins.py -d $drivers/simplix/simplix_mp5  -c $cars $versioning -C $config -r $run simplix_mp5.xml
python check_robot_skins.py -d $drivers/simplix/simplix_sc   -c $cars $versioning -C $config -r $run simplix_sc.xml
python check_robot_skins.py -d $drivers/simplix/simplix_trb1 -c $cars $versioning -C $config -r $run simplix_trb1.xml

# Kilo2008
python check_robot_skins.py -d $drivers/kilo2008/ -c $cars $versioning -C $config -r $run kilo2008.xml

