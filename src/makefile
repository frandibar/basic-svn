# Unix Makefile

CXX      = g++
CXXFLAGS = -ggdb -Wall --pedantic --ansi -fexceptions
INCLUDES = 
LIBS     = -lxerces-c

TARGETS  = svnadmin svnuser contarComponentes ndxReader versionFileReader DVersionsReader

SRCS_common   = Almacen.cpp arbolbmas.cpp ConfigData.cpp Container.cpp DirectoryBlock.cpp DirectoryVersion.cpp DirectoryVersionsFile.cpp FileBlock.cpp File.cpp FileVersion.cpp FileVersionsFile.cpp helper-classes.cpp helpers.cpp nodobmas.cpp nodobmashoja.cpp nodobmasindice.cpp Repositorio.cpp VersionManager.cpp dateindex.cpp datenode.cpp leafdatenode.cpp indexdatenode.cpp datelog.cpp userindex.cpp usernode.cpp indexusernode.cpp leafusernode.cpp UserBlock.cpp UsersRegisterFile.cpp UserLog.cpp
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

OBJS_contarComponentes = $(SRCS_contarComponentes:%.cpp=.objects/%.o)
DPNDS_contarComponentes = $(SRCS_contarComponentes:%.cpp=.objects/%.d)

OBJS_ndxReader = $(SRCS_ndxReader:%.cpp=.objects/%.o)
DPNDS_ndxReader = $(SRCS_ndxReader:%.cpp=.objects/%.d)

OBJS_versionFileReader = $(SRCS_versionFileReader:%.cpp=.objects/%.o)
DPNDS_versionFileReader = $(SRCS_versionFileReader:%.cpp=.objects/%.d)

OBJS_DVersionsReader = $(SRCS_DVersionsReader:%.cpp=.objects/%.o)
DPNDS_DVersionsReader = $(SRCS_DVersionsReader:%.cpp=.objects/%.d)

all: 	$(TARGETS)

.SECONDEXPANSION:
$(TARGETS): 	$$(OBJS_$$@)
	@echo				   
	@echo Making $@...
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) -o $@ $^
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
	rm -rf $(TARGETS)
	rm -rf .depends .objects

tar: 	
	tar -czf svngrupo.tar.gz *.h *.cpp makefile 

-include $(DPNDS_svnadmin) $(DPNDS_svnuser) $(DPNDS_contarComponentes) $(DPNDS_ndxReader) $(DPNDS_versionFileReader) $(DPNDS_DVersionsReader)
