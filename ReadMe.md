## Now Open


_Now Open_ is a reusable application launcher for [Haiku](https://haiku-os.org/) which is capable of opening files
using non-native or command line applications.  The launcher will appear in the standard `Open with...` menu and
`FileTypes` application allowing you to set it as the default handler when opening these types of files from `Tracker`
and other applications.

***Basic instructions are below but the [wiki](../../wiki) contains more details***


### Creating Launchers

To create a new launcher:

1. Duplicate the `Now Open` application file

2. Change any needed options using the `FileTypes` application (signature, supported file types, icon)
		- `Signature` must be unique for every launcher created

3. Double click the launcher to open the settings window where the command can be changed

_You may need to run the launcher once(and possibly reboot) for it to show up in the Haiku `Open with...` menu or
`FileTypes` application_


### Build Instructions

```
~> cd NowOpen
~/NowOpen> jam
```

Use `jam predefined` to build several predefined launchers.
The built launchers will be available in the `obj.X86` directory.

*After the build finishes you may need to run the launcher once(and possibly reboot) for it to show up in the Haiku
`Open with...` menu or `FileTypes` application*


### Creating a new predefined launcher

Make a copy of the [LauncherTemplate.rdef](Source/LauncherTemplate.rdef) file in the [Source](Source) directory and
rename it to the name of your launcher.  Configuration is done by editing a few lines in the rdef file.  You must set
the `app_signature`, the `file_types`, and the `LaunchCommand` resources before building the launcher. You must then
add it to the [Jamfile](Jamfile) with a command like `BuildLauncher MyApp ;`
