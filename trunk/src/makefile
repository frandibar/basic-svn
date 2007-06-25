# Unix Makefile

CXX      = g++
CXXFLAGS = -ggdb -Wall --pedantic --ansi -fexceptions
INCLUDES = 
LIBS     = -lxerces-c
BIN_DIR  = ../bin/

TARGETS         = $(RELEASE_TARGETS)
#TARGETS         = $(RELEASE_TARGETS) $(DEBUG_TARGETS)
RELEASE_TARGETS = svnadmin svnuser 
DEBUG_TARGETS   = contarComponentes ndxReader versionFileReader DVersionsReader

SRCS_common   = Almacen.cpp ConfigData.cpp Container.cpp DirectoryBlock.cpp DirectoryVersion.cpp DirectoryVersionsFile.cpp FileBlock.cpp File.cpp FileVersion.cpp FileVersionsFile.cpp helper-classes.cpp helpers.cpp Repositorio.cpp VersionManager.cpp DateLog.cpp  UserBlock.cpp UsersRegisterFile.cpp Node.cpp VarLenNode.cpp FixLenNode.cpp VarLenIndexNode.cpp VarLenLeafNode.cpp FileDirNode.cpp FileDirIndexNode.cpp FileDirLeafNode.cpp FixLenIndexNode.cpp FixLenLeafNode.cpp bplustree.cpp FixLenBPlusTree.cpp VarLenBPlusTree.cpp FileDirBPlusTree.cpp
SRCS_svnadmin = $(SRCS_common) main-admin.cpp
SRCS_svnuser  = $(SRCS_common) main-user.cpp
SRCS_contarComponentes = contarComponentes.cpp
SRCS_ndxReader = ndxReader.cpp FileVersion.cpp
SRCS_versionFileReader = versionFileReader.cpp
SRCS_DVersionsReader = DVersionsReader.cpp

# TODO: como hacer una regla que expanda OBJS_% ??

OBJS_svnuser  = $(SRCS_svnuser:%.cpp=.objects/%.o)
DPNDS_svnuser = $(SRCS_svnuser:%.cpp=.depends/%.d)

OBJS_svnadmin  = $(SRCS_svnadmin:%.cpp=.objects/%.o)
DPNDS_svnadmin = $(SRCS_svnadmin:%.cpp=.depends/%.d)

OBJS_contarComponentes  = $(SRCS_contarComponentes:%.cpp=.objects/%.o)
DPNDS_contarComponentes = $(SRCS_contarComponentes:%.cpp=.objects/%.d)

OBJS_ndxReader  = $(SRCS_ndxReader:%.cpp=.objects/%.o)
DPNDS_ndxReader = $(SRCS_ndxReader:%.cpp=.objects/%.d)

OBJS_versionFileReader  = $(SRCS_versionFileReader:%.cpp=.objects/%.o)
DPNDS_versionFileReader = $(SRCS_versionFileReader:%.cpp=.objects/%.d)

OBJS_DVersionsReader  = $(SRCS_DVersionsReader:%.cpp=.objects/%.o)
DPNDS_DVersionsReader = $(SRCS_DVersionsReader:%.cpp=.objects/%.d)

all: 	$(TARGETS) 

.SECONDEXPANSION:
$(TARGETS): 	$$(OBJS_$$@)
	@@if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi
	@echo				   
	@echo Making $@...
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) -o $(BIN_DIR)$@ $^
	@echo				   
	@echo Target $@ completed.												   
	@echo				   

.objects/%.o: 	%.cpp
	@@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

.depends/%.d: 	%.cpp
	@@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CXX) $(INCLUDES) -MM -MG $< -MT '.objects/$(@F:%.d=%.o)' > $@

# TODO: el include esta generando los depends innecesariamente
clean: 	
	rm -rf $(TARGETS:%=$(BIN_DIR)%)
	rm -rf .depends .objects

tar: 	
	tar -czf ../svngrupo.tar.gz *.h *.cpp makefile 

-include $(DPNDS_svnadmin) $(DPNDS_svnuser) $(DPNDS_contarComponentes) $(DPNDS_ndxReader) $(DPNDS_versionFileReader) $(DPNDS_DVersionsReader)
