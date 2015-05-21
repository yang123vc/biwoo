**[Index](Index.md)**

---


# 1 BiwooServer Installation #
**`[liunx,win32]`**

## 1.1 Setting ##
**Setting file:**
  1. **$(BIWOO\_PATH)\bin\`[liunx,win32]`\BiwooServer\conf\modules.xml**
    1. **`<name>CommTcpServer</name>`:**
      * Reserve
    1. **`<name>CommUdpServer</name>`:**
      * **`<commport>`:** Main communication port; UDP Default: **8012**
      * **`<capacity>`:** Component communication ability. INTEGER Default: 3
    1. **`<name>FileServer</name>`:**
      * **`<commport>`:** File service transfer port; UDP Default: **8014**
      * **`<capacity>`:** Component communication ability. INTEGER Default: 3
    1. **`<name>CommRtpServer</name>`:**
      * **`<commport>`:** P2P audio and video transfer port; UDP Default: **8020**
      * **`<capacity>`:** Component communication ability. INTEGER Default: 3
    1. **`<name>P2PUDPServer</name>`:**
      * **`<commport>`:** P2P file transfer port; UDP Default: **8022**
      * **`<capacity>`:** Component communication ability. INTEGER Default: 3
  1. **$(BIWOO\_PATH)\bin\`[liunx,win32]`\BiwooServer\conf\biwoo\params.xml**
    1. **`<name>FILESERVER</name>`:**
      * **`<value>`:** FileServer address; PORT or IP:PORT format. Default: **8014**
  1. **$(BIWOO\_PATH)\bin\`[liunx,win32]`\BiwooServer\conf\StreamModule\params.xml**
    1. **`<name>P2PRTPSERVER</name>`:**
      * **`<value>`:** P2P RTP server address; PORT or IP:PORT format. Default: **8020**
    1. **`<name>P2PUDPSERVER</name>`:**
      * **`<value>`:** P2P UDP server address; PORT or IP:PORT format. Default: **8022**

## 1.2 Start BiwooServer ##
**Windows:**
```
cd $(BIWOO_PATH)\bin\wn32\BiwooServer
win32_run.bat
```

**Linux:**
```
# cd $(BIWOO_PATH)\bin\linux\BiwooServer
# ./CGCP
```

# 2 BiwooClient Installation #
**`[win32]`**

## 2.1 Setting ##
**Setting file:**
  * **Windows:** $(BIWOO\_PATH)\win32\BiwooClient\default\setting.xml

**Network setting:**
  * **`<address>`:** Main communication port; UDP Default: **ip:8012**
  * ~~**`<fileserver>`:** File service transfer port; UDP Default: ip:8014~~
  * ~~**`<p2p_rtp>`:** P2P audio and video transfer port; UDP Default: ip:8020~~
  * ~~**`<p2p_udp>`:** P2P file transfer port; UDP Default: ip:8022~~

**Locale and language setting:**
  * **`<locale>`:**
  * **`<langtext>`:** langtext\_en.xml,langtext\_ch.xml

## 2.2 Run BiwooClient ##
  * **Windows:** Run $(BIWOO\_PATH)\bin\win32\BiwooClient\biwooclient.exe