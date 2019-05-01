# jvulkan-natives-Linux-x86_64

**jvulkan-natives-Linux-x86_64** is a native library written in c++ that is used (required) by the 
[jvulkan](https://github.com/dkaip/jvulkan) (Java Vulkan SDK) project.  This project provides the interface between **jvulkan** 
and the [Vulkan® SDK](https://www.lunarg.com/vulkan-sdk/) from LUNARG which is written in c++.

## Prerequisites
You will need the [Vulkan® SDK](https://www.lunarg.com/vulkan-sdk/) from LUNARG.  You will 
need to download and build it.  If possible get the version <code>1.1.101.0</code>.  Later 
**might** work as well.

You will also need the Wayland development code.  It may be in the package 
<code>wayland-devel.x86\_64</code> and / or perhaps <code>libwayland-client.x86\_64</code>. 
Additionally you may just be able to download it from [freedesktop.org](https://wayland.freedesktop.org/).

## Get the code
Use the <code>git clone</code> command to get the code. 

<code>git https://github.com/dkaip/jvulkan-natives-Linux-x86_64 jvulkan-natives-Linux-x86_64</code> 

This will create a <code>jvulkan-natives-Linux-x86_64</code> directory in your current directory


## Building
Change your current directory (<code>cd</code>) to the project root directory.  If you used the 
command above it would be <code>cd jvulkan-natives-Linux-x86_64</code>.    

Edit the <code>CMakeLists.txt</code> file.</br>
Alter the second arguments of the <code>set</code> statements that are 
before the <code>project</code> statement and replace the values that are 
currently there with values that are appropriate for your system / development 
environment.
<p>
Just as a side note, I was not able to debug the native code with a debugger while 
developing it.  Possibly since this code's job is to interface between code written
in Java and the libraries provided in the LunarG Vulkan SDK. I had to debug the old fashioned way with print statements (<code>cout</code>), although now slf4j logging is available via the MACROs in the header file 
<code>slf4j.hh</code>.  These MACROs are <code>LOGINFO</code>, <code>LOGDEBUG</code>, <code>LOGTRACE</code>, 
<code>LOGWARN</code>, <code>LOGERROR</code>. The <code>Logger</code> used is 
<code>&quot;jvulkan-natives&quot;</code> 
so make sure it is configured into your <code>log4j2.xml</code>.

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

In both of these cases the file of interest is <code>libjvulkan-natives-Linux-x86_64.so</code>.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.  

Please do not reformat the source code for the existing code.  If you make additions, go ahead and format 
the source files as you like.

Please make sure to update tests as appropriate.  There are none at the moment for this project.

In the event you are upgrading or adding to this software the command to recreate the 
JNI header file for the native functions is:

<code>javah -classpath my-jvulkan-project-path/src/main/java com.CIMthetics.jvulkan.VulkanCore.VK11.NativeProxies</code> 

This will create the file <code>com&lowbar;CIMthetics&lowbar;jvulkan&lowbar;VulkanCore&lowbar;VK11&lowbar;NativeProxies.h</code> 
that will need to be placed in the <code>headers</code> directory of 
the **jvulkan-natives-Linux-x86_64** project on your machine.  You will then need to implement 
any new functions you have added here in that project as well. 

The value of &quot;my-jvulkan-project-path&quot; will be the path to where you have the **jvulkan** project stored 
on your machine.

## License
[Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0)