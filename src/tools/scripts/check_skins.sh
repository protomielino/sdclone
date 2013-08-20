
# Script to scan through all robots

# Uncommet for SVN version checking
versioning="-s"
# of for Git-svn version checking
#versioning="-g"

# Process all files regardless of versions
#versioning="-a"

drivers="`pwd`/../../../data/drivers"
cars="`pwd`/../../../data/cars/models/"

# Enable the screen shots
config="-C $HOME/.speed-dreams-2"
run="-r /usr/local/games/speed-dreams-2"

# Use alternative script to process images
#proc="-p my_script.sh"

# Check the car models (for humans)
python check_car_skins.py -c $cars $versioning $config $run $proc

# Usr
python check_robot_skins.py -d $drivers/usr_36GP -c $cars $versioning $config $run $proc usr_36GP.xml
python check_robot_skins.py -d $drivers/usr_ls1  -c $cars $versioning $config $run $proc usr_ls1.xml
python check_robot_skins.py -d $drivers/usr_ls2  -c $cars $versioning $config $run $proc usr_ls2.xml
python check_robot_skins.py -d $drivers/usr_mpa1 -c $cars $versioning $config $run $proc usr_mpa1.xml
python check_robot_skins.py -d $drivers/usr_sc   -c $cars $versioning $config $run $proc usr_sc.xml
python check_robot_skins.py -d $drivers/usr_trb1 -c $cars $versioning $config $run $proc usr_trb1.xml
python check_robot_skins.py -d $drivers/usr_rs   -c $cars $versioning $config $run $proc usr_rs.xml

# Simplix
python check_robot_skins.py -d $drivers/simplix_36GP -c $cars $versioning $config $run $proc simplix_36GP.xml
python check_robot_skins.py -d $drivers/simplix_ls1  -c $cars $versioning $config $run $proc simplix_ls1.xml
python check_robot_skins.py -d $drivers/simplix_ls2  -c $cars $versioning $config $run $proc simplix_ls2.xml
python check_robot_skins.py -d $drivers/simplix_mp5  -c $cars $versioning $config $run $proc simplix_mp5.xml
python check_robot_skins.py -d $drivers/simplix_sc   -c $cars $versioning $config $run $proc simplix_sc.xml
python check_robot_skins.py -d $drivers/simplix_trb1 -c $cars $versioning $config $run $proc simplix_trb1.xml
python check_robot_skins.py -d $drivers/simplix_mpa1 -c $cars $versioning $config $run $proc simplix_mpa1.xml

# Kilo2008
python check_robot_skins.py -d $drivers/kilo2008/ -c $cars $versioning $config $run $proc kilo2008.xml

