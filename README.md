![Lucid](/Branding/logo-black.png?raw=true "Lucid")

Hey guys,

After building the solution please navigate to the following folder directory:

Lucid > vendor > assimp

You will find the assimp.dll file here. You will need to copy and paste this into the following directory:

bin > Debug > Lucid

There is also a Release configuration I created for the project. This runs the project with optimizations. Feel free to select this configuration for use within Visual Studio if you wish for the application to run slightly quicker than it would in Debug mode.

If you run into linker errors that are complaining about a library called YAML, simply select Rebuild All from the Build menu in Visual Studio. This is a known issue with the project, due to time constraints I couldn't investigate it further to provide a fix

I have also had to zip the entire project directory to adhere to the file size limit Moodle imposes. You'll have to unzip this to gain access to the source code files.

NOTE: If you do build in Release mode please follow the above step of copying the assimp.dll into the Release folder directory.

Any other unexpected issues you may have building or running the project please email me.

Enjoy Lucid Engine :D