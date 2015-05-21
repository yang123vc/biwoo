[Index](Index.md)

---


# Linux Compile #
**!!! Only support BiwooServer linux compile !!!**

| # ./configure CPPFLAGS="-I/usr/local/include/jrtplib3 -I/usr/local/include/jthread" |
|:------------------------------------------------------------------------------------|
| # make                                                                              |
| # sudo make install                                                                 |

# Windows Compile #
  * **ThirdParty compile:**
Open $(BIWOO\_PATH)\src\ThirdParty\proj\MSVC8.0\libThirdParty.sln file and build solution.
  * **BiwooServer compile:**
Open $(BIWOO\_PATH)\src\proj\MSVC8.0\biwooserver.sln file and build solution.
  * **BiwooClient compile:**
Open $(BIWOO\_PATH)\src\proj\MSVC8.0\biwooclient.sln file and build solution.