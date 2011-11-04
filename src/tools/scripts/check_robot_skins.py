
import os
import glob
from xml.etree.ElementTree import ElementTree
from xml.etree.ElementTree import SubElement
from optparse import OptionParser

try:
	import pysvn
	_has_pysvn = True
except ImportError:
	_has_pysvn = False

try:
	from git import *
	_has_pygit = True
except ImportError:
	_has_pygit = False

try:
	import Image
	_has_PIL = True
except ImportError:
	_has_PIL = False

parser = OptionParser()

parser.set_defaults(dir=".", cars=".", config=None, run=None, svn=None, git=None, proc=None, all=None)
parser.add_option("-d",  "--dir", dest="dir", help="driver directory")
parser.add_option("-c",  "--cars", dest="cars", help="cars directory")
parser.add_option("-C",  "--config", dest="config", help="path to '.speed-dreams' config directory")
parser.add_option("-r",  "--run", dest="run", help="command to run SpeedDreams")
parser.add_option("-p",  "--proc", dest="proc", help="command to process preview images")
parser.add_option("-a",  "--all", action="store_true", dest="all", help="process all previews regardless")

if _has_pysvn:
	parser.add_option("-s", "--svn", action="store_true", dest="svn", help="report svn version numbers")
if _has_pygit:
	parser.add_option("-g", "--git", action="store_true", dest="git", help="report git verison numbers")

(options, args) = parser.parse_args()

def check_version(myfile):
	# Check myfile exists
	if not os.access(myfile, os.R_OK):
		return None

	# Return SVN revision
	if _has_pysvn and options.svn:
		client = pysvn.Client()
		entry = client.info(myfile)
		if entry:
			return entry.commit_revision.number
		else:
			return -1

	# Return GIT revision
	if _has_pygit and options.git:
		repo = Repo(myfile)
		commits = repo.commits(path=myfile, max_count=1)
		if commits:
			for line in commits[0].message.splitlines():
				if line.startswith("git-svn-id:"):
					return int(line.split("@", 1)[1].split(" ",1)[0])
		else;
			return -1

	# Fall through when SVN/GIT not present
	return 1

#---

def get_screenshot(index, car, skin):
	if not options.config or not options.run:
		return None

	if options.all:
		skin_done = True
	else:
		skin_done = False

	config_file = os.sep.join([options.config, "config/raceman/practice.xml"])

	if not os.access(config_file, os.R_OK):
		print "Can't find 'practice.xml' (", config_file, ","
		return None

	module = os.path.splitext(args[0])[0]

	my_ele = ElementTree()
	p = my_ele.parse(config_file)
	q = p.findall("section")

	for i in list(q):
		if i.attrib["name"] == "Drivers" or i.attrib["name"] == "Drivers Start List":
			for k in list(i):
				if k.attrib["name"] == "focused module":
					k.set("val", module)
				if k.attrib["name"] == "focused idx":
					k.set("val", index)

			j = i.find("section")
	
			# modify attributes
			for k in list(j):
				if k.attrib["name"] == "idx":
					k.set("val", index)
				if k.attrib["name"] == "module":
					k.set("val", module)
				if k.attrib["name"] == "skin name":
					skin_done = True
					if skin:
						k.set("val", skin)
					else:
						j.remove(k)

			if not skin_done and skin:
				SubElement(j, "attstr", {'name':"skin name", 'val':skin})

			'''
			# dump attributes
			for k in list(j):
				print ":", k.attrib["name"], k.attrib["val"]
			'''

		if i.attrib["name"] == "Driver Info":
			j = i.find("section/section/section")

			for k in list(j):
				if k.attrib["name"] == "car name":
					k.set("val", car)

	# Store the changes
	my_ele.write(config_file)

	# Run the game
	os.system(options.run)

	# return the filename of screen shot

#---

print "Checking", args[0]
print "---"

tree = ElementTree().parse(os.sep.join([options.dir, args[0]]))

p = tree.find("section/section")

for item in list(p):
	number = item.attrib["name"]

	for driver in list(item):
		if (driver.attrib["name"] == "name"):
			name = driver.attrib["val"]
		
		if (driver.attrib["name"] == "car name"):
			car = driver.attrib["val"]
		
	print number, ":", name, "(", car, ")"

	# Check acc model
	model = ".".join([os.sep.join([options.cars, car, car]), "acc"])
	model_ver = check_version(model)

	if (model_ver == None):
		print "ACC model is missing"
		print
		continue

	# Check for 'Standard Skin'
	standard = ".".join([os.sep.join([options.dir, number, car]), "png"])
	standard_ver = check_version(standard)

	if standard_ver:
		preview = "-".join([os.sep.join([options.dir, number, car]), "preview.jpg"])
		preview_ver = check_version(preview)
		screenshot = None

		if (model_ver > standard_ver):
			print "Standard: ACC Model is newer"
		else:
			print "Standard: OK"

		if (preview_ver == None):
			print "Preview : Missing"
			screenshot = True
		elif (preview_ver < 0):
			print "Preview : Not in version control"
		elif (preview_ver < standard_ver):
			print "Preview : Out of date"
			screenshot = True
		else:
			print "Preview : OK"

		if options.config and options.run and screenshot:
			screenshot = get_screenshot(number, car, None)

			if options.proc:
				# Call alternative script to process images
				os.system(" ".join([options.run, preview]))
			elif _has_PIL:
				screenshot_files = os.listdir(os.sep.join([options.config,"screenshots"]))
				if screenshot_files:
					screenshot_file = screenshot_files[0]

					screenshot = Image.open(os.sep.join([options.config, "screenshots", screenshot_file]))
					scaled = screenshot.resize((800,500), Image.ANTIALIAS)
					# scaled.MAXBLOCK=scaled.size[0]*scaled.size[1]

					scaled.save(preview, quality=90, optimize=True, subsampling='4:4:4')
					os.remove(os.sep.join([options.config, "screenshots", screenshot_file]))
	else:
		print "Standard: Missing"

	# Check for alternate skins (specific for driver)
	alternates = glob.glob("-".join([os.sep.join([options.dir, number, car]), "*.png"]))
	if (alternates != None):
		for alternate in alternates:
			alternate_ver=check_version(alternate)
			screenshot = None

			if (model_ver > alternate_ver):
				print "Alternate:", os.path.basename(alternate), "ACC Model is newer"

			if (alternate_ver != None):
				(filename,ext) = os.path.splitext(alternate)
				preview = "-".join([filename, "preview.jpg"])
				preview_ver = check_version(preview)
				
				print "Alternate:", os.path.basename(alternate)
				if (preview_ver == None):
					print "Preview : Missing"
					screenshot = True
				elif (preview_ver < 0):
					print "Preview : Not in version control"
				elif (preview_ver < standard_ver):
					print "Preview : Out of date"
					screenshot = True
				else:
					print "Preview : OK"

				if options.config and options.run and screenshot:
					screenshot = get_screenshot(number, car, filename.rsplit("-",1)[1] )

					if options.proc:
						# Call alternative script to process images
						os.system(" ".join([options.run, preview]))
					elif _has_PIL:
						screenshot_files = os.listdir(os.sep.join([options.config,"screenshots"]))
						if screenshot_files:
							screenshot_file = screenshot_files[0]

							screenshot = Image.open(os.sep.join([options.config, "screenshots", screenshot_file]))
							scaled = screenshot.resize((800,500), Image.ANTIALIAS)
							# scaled.MAXBLOCK=scaled.size[0]*scaled.size[1]

							scaled.save(preview, quality=90, optimize=True, subsampling='4:4:4')
							os.remove(os.sep.join([options.config, "screenshots", screenshot_file]))


	print

