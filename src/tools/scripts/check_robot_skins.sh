
# Script to scan through all robots

# Uncommet for SVN version checking
#versioning="-s"
# of for Git-svn version checking
#versioning="-g"

drivers="`pwd`/../../drivers/"
cars="`pwd`/../../../data/cars/models/"


# usr
python check_robot_skins.py -d $drivers/usr/usr_36GP -c $cars $versioning usr_36GP.xml
python check_robot_skins.py -d $drivers/usr/usr_ls1  -c $cars $versioning usr_ls1.xml
python check_robot_skins.py -d $drivers/usr/usr_ls2  -c $cars $versioning usr_ls2.xml
python check_robot_skins.py -d $drivers/usr/usr_sc   -c $cars $versioning usr_sc.xml
python check_robot_skins.py -d $drivers/usr/usr_trb1 -c $cars $versioning usr_trb1.xml
python check_robot_skins.py -d $drivers/usr/usr_rs   -c $cars $versioning usr_rs.xml

# Simplix
python check_robot_skins.py -d $drivers/simplix/simplix_36GP -c $cars $versioning simplix_36GP.xml
python check_robot_skins.py -d $drivers/simplix/simplix_ls1  -c $cars $versioning simplix_ls1.xml
python check_robot_skins.py -d $drivers/simplix/simplix_ls2  -c $cars $versioning simplix_ls2.xml
python check_robot_skins.py -d $drivers/simplix/simplix_mp5  -c $cars $versioning simplix_mp5.xml
python check_robot_skins.py -d $drivers/simplix/simplix_sc   -c $cars $versioning simplix_sc.xml
python check_robot_skins.py -d $drivers/simplix/simplix_trb1 -c $cars $versioning simplix_trb1.xml

# Kilo2008
python check_robot_skins.py -d $drivers/kilo2008/ -c $cars $versioning kilo2008.xml
