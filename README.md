# sc_open

**sc_open** is the Station Controller (sc) application originally created by 
John F. Poirier DBA Edge Integration circa early this century or late in the previous one. 
It has been updated to run properly in 64-bit software environments.

The primary purpose of this application is to be able to communicate with equiment that
is on the factory floor in a semiconductor manufacturing facility.   If you do not work 
in that industry or you are not familiar with the SEMI Standards E4, E5, E30, E37, 
el al, this application is probably not for you.  

This software functions well in development environments as well as production ones.  

In a development environment this software enables quick and mostly painless 
communications with equipment commonly found in a factory for making semiconductor 
devices.  This communication is done primarily using the SECSII protocol transported via 
either a SECSI or HSMS transport layer.  This application is a great tool to test out
early operational scenarios with a piece of equipment and to generate log file(s).  The
log files can usually be merged, with little to moderate work, with a program for 
<code>sc</code> in order to 
create an equipment simulator.  This created simulator is an invaluable tool for use 
in the job of creating and debugging equipment interfaces developed in another 
platform / software tool. In addition, once a tool is in production a <code>sc</code> 
program can be created from production log files to allow a &quot;production&quot; equipment 
interface to be run against a fake version of the tool for debugging purposes.  This 
allows for testing / debugging without putting product at risk. 

Two running instances of <code>sc</code> can communicate with each other.  This is sometimes 
used to develop both a host and an equipment interface at the same time.

In a production this application can be used to construct a robust automation layer for
simpler CIM environments.  It is probably not going to be used in production in a 300mm or 
450mm environment due to the massive amount of automation framework up and down the 
application stack.


## Get the code
Use the <code>git clone</code> command to get the code. 

<code>git https://github.com/dkaip/sc_open sc_open</code> 

This will create a <code>sc_open</code> directory in your current directory


## Building
Change your current directory (<code>cd</code>) to the project root directory.  If you used the 
command above it would be <code>cd sc_open</code>.    

Edit the <code>CMakeLists.txt</code> file.</br>
Alter the second argument of the line <code>set(NormalSystemLibraries "/usr/lib64")</code> statement 
that is before the <code>project(sc)</code> statement and replace the value that is 
currently there with value that is appropriate for your system / development 
environment.

To produce a debug version:

<code>
mkdir Debug </br>
cd Debug </br>
cmake -DCMAKE_BUILD_TYPE=Debug .. </br>
make -j32 (You may leave out the -j option it just tells make how many cores it may use.) </code>

To produce a release version:

<code>
mkdir Release</br>
cd Release</br>
cmake -DCMAKE_BUILD_TYPE=Release ..</br>
make -j32 (You may leave out the -j option it just tells make how many cores it may use.)</br> </code>

In both of these cases the file of interest is <code>sc</code>. 

## Documentation
If you look in the <code>docs</code> directory you will see a file named 
<code>sc&lowbar;users&lowbar;guide.odt</code>.  This file is the current version of the documentation. 
At the moment it is incomplete because the original editable version of the documentation 
is missing.  The file <code>sc&lowbar;users&lowbar;guide.pdf</code> is the original documentation in pdf 
format. <code>sc&lowbar;users&lowbar;guide.odt</code> is currently being updated from this source.  Once 
the process is complete a new version of <code>sc&lowbar;users&lowbar;guide.pdf</code> will be created 
that will then be able to be updated as necessary.

## Example sc programs
If you look in the <code>examples</code> directory you will find some example programs 
to peruse.  The example programs have a file extension of <code>.sc</code>, but, they 
can use any extension name or even no extension name.

# Running sc
Refer to the **Getting Started** section in the documentation for a complete description 
of how you run <code>sc</code>.  

A very common way to start it in a development environment is with the command: 
<br>
<code>sc -i -c &quot;read my&lowbar;program&lowbar;file&quot;</code> 

This will start <code>sc</code> which will attemp to load its &quot;program file&quot; file 
that has a name of <code>my&lowbar;program&lowbar;file</code>...in this case a name without an extension.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.  

