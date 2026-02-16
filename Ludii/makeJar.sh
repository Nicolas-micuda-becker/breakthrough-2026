rm -f RandPlayerLocal.jar
javac -cp Ludii-1.3.14.jar RandPlayerLocal.java
mv RandPlayerLocal.class bk/
jar cf RandPlayerLocal.jar bk/RandPlayerLocal.class
