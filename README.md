# RS_EOS_Astro <br/>
server for remote control APN  <br/>
compatible with https://github.com/jeromefavrou/Intervallometre <br/>

# Installation <br/>

you can use directly release version for linux vesion <br/>
 - Ubuntu xi86: <br/>
  -> wget https://github.com/jeromefavrou/RS_EOS_Astro/releases/download/V_01.01/RS_EOS_Astro <br/>
 - Raspbian ARM x64: <br/>
  -> wget https://github.com/jeromefavrou/RS_EOS_Astro/releases/download/rasp_V_01.0/RS_EOS_Astro <br/>
 
and finished whith <br/>
 -> chmod +x ./RS_EOS_Astro <br/>
 
and run the programme <br/>
 -> ./RS_EOS_Astro <br/>
 
the essentials files will be created in the first run <br/>
 
also you can compile source code on your linux version <br/>
the INSTALL file can be edit <br/>

 -> git clone https://github.com/jeromefavrou/RS_EOS_Astro.git <br/>
 
 -> cd RS_EOS_Astro <br/>
 
 -> chmod +x ./INSTALL <br/>
  
 -> ./INSTALL <br/>
  choose the gphoto2 install mode and wait a moment <br/>
  for finish go to /home/user/RS_EOS_Astro <br/>
  
 -> cd ~/RS_EOS_Astro <br/>
 
  and run the programme <br/>
 -> ./RS_EOS_Astro <br/>
 
 # Configuration <br/>
 2 essentials files are used by RS_EOS_Astro (will be created in the first run RS_EOS_Astro but warning relative directory)<br/>
 
 .mnt_configure <br/>
  - configure the methode for mount device <br/>
  - configure the path of mount <br/>
  
 .server_configure <br/>
  - Configure the port of server <br/>
  
  for most information go on the wiki : https://github.com/jeromefavrou/RS_EOS_Astro/wiki <br/>
 
 # WARNING
 don't run INSTALL on sudo <br/>
 don't run RS_EOS_Astro on sudo <br/>
 RS_EOS_Astro use relative directory so "./RS_EOS_Astro/RS_EOS_Astro" will be different result than "cd RS_EOS_Astro"  "./RS_EOS_Astro " <br/>
 this second option is recommended <br/>
 
 
 
