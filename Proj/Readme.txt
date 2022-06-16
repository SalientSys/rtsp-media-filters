///
/// SalientSys.live555-3rdPartySources nuget
///


///
/// Contents: ?????
///
- Contains 3rd party source code that provides support to live555 RTSP-server implementation in Recording Server.


///
/// Note 
///
- Some of the files are ours but moved here for dependency issues & related code being together.


///
/// FiltersMediaSources-Proj    ************** Create this folder ****************
///
- This folder contains the VS project files that were needed to build the .lib files


///
/// Dependency (consumed all as nuget)
///
- live555.v142.mt
- Intel-tbb
- openssl 


///
/// Build
///
- VS2019, v142 toolset.
- OutputDir: In $(Configuration)\$(Platform)
- TargetName: Follows $(Configuration)\$(Platform)