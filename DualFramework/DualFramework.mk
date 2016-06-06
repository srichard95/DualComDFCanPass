FRAMEWORKRLIB = ./DualFramework
# List of all the Userlib files
FRAMEWORKSRC =  $(FRAMEWORKRLIB)/src/DataLinkLayer.c \
				$(FRAMEWORKRLIB)/src/NetworkLayer.c \
				$(FRAMEWORKRLIB)/src/crc.c \
          
# Required include directories
FRAMEWORKINC =  $(FRAMEWORKRLIB) \
                $(FRAMEWORKRLIB)/include 
