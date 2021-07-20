all : scube tcm gss

.PHONY : all
CXX = g++
CFLAGS = -D DEBUG -std=c++11

scube : scube.o
	$(CXX) -o scube scube.o $(CFLAGS)
tcm : tcm.o
	$(CXX) -o tcm tcm.o $(CFLAGS)
gss : gss.o
	$(CXX) -o gss gss.o $(CFLAGS)


scube.o : ScubeCode/main.cpp
	$(CXX) -o scube.o -c ScubeCode/main.cpp -D SR8 -D H0 -D ROOM2 $(CFLAGS)
tcm.o : BaselineCode/TCM.cpp
	$(CXX) -o tcm.o -c BaselineCode/TCM.cpp $(CFLAGS)
gss.o : BaselineCode/GSS.cpp
	$(CXX) -o gss.o -c BaselineCode/GSS.cpp $(CFLAGS)
	

.PHONY:clean
clean:
	-$(RM) scube tcm gss
	-$(RM) scube.o tcm.o gss.o

