all : gss tcm
b1 : gss
b2 : tcm

.PHONY : all
CXX = g++
CFLAGS = -g -D DEBUG -std=c++11 -static-libstdc++ -lpthread

gss : gss.o
	$(CXX) -o gss gss.o $(CFLAGS)

tcm : tcm.o
	$(CXX) -o tcm tcm.o $(CFLAGS)

gss.o : GSS.cpp
	$(CXX) -o gss.o -c GSS.cpp $(CFLAGS)

tcm.o : TCM.cpp
	$(CXX) -o tcm.o -c TCM.cpp $(CFLAGS)



.PHONY : clean
clean:
	-$(RM) gss tcm
	-$(RM) gss.o tcm.o

.PHONY : rm-gss
rm-gss:
	-$(RM) gss
	-$(RM) gss.o

.PHONY : rm-tcm
rm-tcm:
	-$(RM) tcm
	-$(RM) tcm.o