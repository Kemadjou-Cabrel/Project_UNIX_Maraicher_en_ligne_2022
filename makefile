.SILENT:

OBS = g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_DOSSIER_FINAL -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o

OBS2 = g++ -Wno-unused-parameter -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../Administrateur -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++

all:	AccesBD Caddie Publicite CreationBD Serveur Client Gerant

AccesBD:	AccesBD.cpp
	echo "Creation du AccesBD"
	g++ -o AccesBD AccesBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

Caddie:	Caddie.cpp
	echo "Creation du Caddie"
	g++ -o Caddie Caddie.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

Publicite:	Publicite.cpp
	echo "Creation du Publicite"
	g++ -o Publicite Publicite.cpp

CreationBD:	CreationBD.cpp
	echo "Creation du CreationBD"
	g++ -o CreationBD CreationBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

Serveur:	Serveur.cpp FichierClient.o MySemaphores.o
	echo "Creation du Serveur"
	g++ Serveur.cpp FichierClient.o MySemaphores.o -o Serveur -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

FichierClient.o:	FichierClient.cpp
	echo "creation du FichierClient.o"
	g++ -c FichierClient.cpp

MySemaphores.o:	MySemaphores.h MySemaphores.cpp
	echo "Creation du MySemaphores.o"
	gcc -c MySemaphores.cpp

Client:	mainclient.o windowclient.o moc_windowclient.o
	echo "Creation du Client"
	g++ -Wno-unused-parameter -o Client mainclient.o windowclient.o moc_windowclient.o  /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread

moc_windowclient.o:	moc_windowclient.cpp
	echo "Creation du moc_windowclient.o"
	$(OBS) moc_windowclient.o moc_windowclient.cpp

windowclient.o:	windowclient.cpp
	echo "Creation du windowclient.o"
	$(OBS) windowclient.o windowclient.cpp

mainclient.o:	mainclient.cpp
	echo "Creation du mainclient.o"
	$(OBS) mainclient.o mainclient.cpp

Gerant:	maingerant.o windowgerant.o moc_windowgerant.o MySemaphores.o
	echo "Creation du Gerant"
	g++ -Wno-unused-parameter -o Gerant maingerant.o windowgerant.o moc_windowgerant.o MySemaphores.o /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

moc_windowgerant.o:	moc_windowgerant.cpp
	echo "Creation du moc_windowgerant.o"
	$(OBS2) -o moc_windowgerant.o moc_windowgerant.cpp

windowgerant.o:	windowgerant.cpp
	echo "Creation du windowgerant.o"
	$(OBS2) -I/usr/include/mysql -m64 -L/usr/lib64/mysql -o windowgerant.o windowgerant.cpp

maingerant.o:	maingerant.cpp
	echo "Creation du maingerant.o"
	$(OBS2) -o maingerant.o maingerant.cpp

clean:
	rm *.o
