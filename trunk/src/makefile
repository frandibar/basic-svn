# Unix Makefile

CXX      = g++
CXXFLAGS = -ggdb -Wall --pedantic --ansi -fexceptions
INCLUDES = 
LIBS     = -lxerces-c

TARGETS  = svnadmin svnuser 

SRCS_common = Almacen.cpp arbolbmas.cpp bloque.cpp ConfigData.cpp helper-classes.cpp nodobmas.cpp nodobmashoja.cpp nodobmasindice.cpp Repositorio.cpp version.cpp versionFile.cpp VersionManager.cpp
SRCS_svnadmin = $(SRCS_common) main-admin.cpp
SRCS_svnuser  = $(SRCS_common) main-user.cpp

# TODO: como hacer una regla que expanda OBJS_% ??

OBJS_svnuser  = $(SRCS_svnuser:%.cpp=.objects/%.o)
DPNDS_svnuser = $(SRCS_svnuser:%.cpp=.depends/%.d)

OBJS_svnadmin  = $(SRCS_svnadmin:%.cpp=.objects/%.o)
DPNDS_svnadmin = $(SRCS_svnadmin:%.cpp=.depends/%.d)

all: 	$(TARGETS)

.SECONDEXPANSION:
$(TARGETS): 	$$(OBJS_$$@)
	@echo				   
	@echo Making $@...
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) -o $@ $^
	@echo Target $@ completed.												   

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

-include $(DPNDS_svnuser) $(DPNDS_svnuser)

