import os
import glob
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

parser = OptionParser()

parser.set_defaults(cars=".", svn=None, git=None)
parser.add_option("-c",  "--cars", dest="cars", help="cars directory")
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
                return entry.commit_revision.number

        # Return GIT revision
        if _has_pygit and options.git:
                repo = Repo(myfile)
                for line in repo.commits(path=myfile, max_count=1)[0].message.splitlines():
                        if line.startswith("git-svn-id:"):
                                return int(line.split("@", 1)[1].split(" ",1)[0])

        # Fall through when SVN/GIT not present
        return 1



def check_dir(args, dirname, names):
	car =  os.path.basename(dirname)

	for name in names:
		(root, ext) = os.path.splitext(name)
		if root == car and ext == ".xml":
			# Found config file
			print "checking", root
			print "---"

			# Check acc model
			model = ".".join([os.sep.join([options.cars, car, car]), "acc"])
			model_ver = check_version(model)

			if (model_ver == None):
				print "ACC model is missing"
				print
				return


			# Checking for standard
			standard = ".".join([os.sep.join([options.cars, car, car]), "png"])
			standard_ver = check_version(standard)

			if standard_ver:
				preview = "-".join([os.sep.join([options.cars, car, car]), "preview.jpg"])
				preview_ver = check_version(preview)

				if (model_ver > standard_ver):
					print "Standard: ACC Model is newer"
				else:
					print "Standard: OK"

				if (preview_ver == None):
					print "Preview : Missing"
				elif (preview_ver < standard_ver):
					print "Preview : Out of date"
				else:
					print "Preview : OK"
			else:
				print "Standard: Missing"

			# Check for alternate skins (specific for driver)
			alternates = glob.glob("-".join([os.sep.join([options.cars, car, car]), "*.png"]))

			if (alternates != None):
				for alternate in alternates:
					alternate_ver=check_version(alternate)

					if (model_ver > alternate_ver):
						print "Alternate:", os.path.basename(alternate), "ACC Model is newer"

					if (alternate_ver != None):
						(preview,ext) = os.path.splitext(alternate)
						preview_ver = check_version("-".join([preview, "preview.jpg"]))

						print "Alternate:", os.path.basename(alternate)
						if (preview_ver == None):
							print "Preview : Missing"
						elif (preview_ver < standard_ver):
							print "Preview : Out of date"
						else:
							print "Preview : OK"

			print

#---

os.path.walk(options.cars, check_dir, "")
