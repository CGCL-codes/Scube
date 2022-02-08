all : gss.r tcm.r
b1 : gss.r
b2 : tcm.r

.PHONY : all
CXX = g++
CFLAGS = -g -D DEBUG -std=c++11 -static-libstdc++ -lpthread

gss.r : gss.o
	$(CXX) -o gss.r gss.o $(CFLAGS)

tcm.r : tcm.o
	$(CXX) -o tcm.r tcm.o $(CFLAGS)

gss.o : GSS.cpp
	$(CXX) -o gss.o -c GSS.cpp $(CFLAGS)

tcm.o : TCM.cpp
	$(CXX) -o tcm.o -c TCM.cpp $(CFLAGS)



.PHONY : clean
clean:
	-$(RM) gss.r tcm.r
	-$(RM) gss.o tcm.o

.PHONY : rm-gss
rm-gss:
	-$(RM) gss.r
	-$(RM) gss.o

.PHONY : rm-tcm
rm-tcm:
	-$(RM) tcm.r
	-$(RM) tcm.o