project(soundkonverter)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

#find_package(Cdparanoia)
#macro_log_feature(CDPARANOIA_FOUND "cdparanoia" "Reads digital audio directly from a CD, then writes the data to a file or pipe in several different formats" "http://www.xiph.org/paranoia/" FALSE "" "Required for building soundKonverter.")

#include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} /usr/include/taglib ${CDPARANOIA_INCLUDE_DIR})
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} /usr/include/taglib)

add_subdirectory(plugins/soundkonverter_codec_lame)
add_subdirectory(plugins/soundkonverter_codec_ogg)
add_subdirectory(plugins/soundkonverter_codec_ffmpeg)
add_subdirectory(plugins/soundkonverter_replaygain_vorbisgain)
add_subdirectory(plugins/soundkonverter_ripper_cdparanoia)

set(soundkonvertercore_SRCS
   core/backendplugin.cpp
   core/codecplugin.cpp
   core/codecwidget.cpp
   core/conversionoptions.cpp
   core/replaygainplugin.cpp
   core/ripperplugin.cpp
)

kde4_add_library(soundkonvertercore SHARED ${soundkonvertercore_SRCS})

target_link_libraries(soundkonvertercore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
#set_target_properties(soundkonvertercore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS soundkonvertercore DESTINATION ${LIB_INSTALL_DIR})


# add_definitions(${TAGLIB_CFLAGS} "${TAGLIB_CFLAGS}/..")
# ADD_DEFINITIONS(${TAGLIB_CFLAGS} -DQT_STL)

set(soundkonverter_SRCS
   soundkonverter.cpp
   main.cpp
   soundkonverterview.cpp
   config.cpp
   filelistmodel.cpp
   filelist.cpp
   filelistitem.cpp
   combobutton.cpp
   progressindicator.cpp
   options.cpp
   optionssimple.cpp
   optionsdetailed.cpp
   optionslayer.cpp
   optionseditor.cpp
   pluginloader.cpp
   configdialog.cpp
   configpagebase.cpp
   configgeneralpage.cpp
   configadvancedpage.cpp
   configbackendspage.cpp
   outputdirectory.cpp
   convert.cpp
   logger.cpp
   logviewer.cpp
   metadata/tagengine.cpp
   opener.cpp
   dirdialog.cpp
   conversionoptionsmanager.cpp
   global.cpp
   audiocd/cdmanager.cpp
   audiocd/paranoia.cpp
   audiocd/cdopener.cpp
   replaygainscanner.cpp
   replaygainfilelist.cpp
   replaygainfilelistitem.cpp
 )

#kde4_automoc(${soundkonverter_SRCS})

#kde4_add_ui_files(soundkonverter_SRCS soundkonverterview_base.ui prefs_base.ui)

#kde4_add_kcfg_files(soundkonverter_SRCS settings.kcfgc )

kde4_add_executable(soundkonverter ${soundkonverter_SRCS})

# target_link_libraries(soundkonverter ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} /usr/lib/libtag.so /usr/lib/libmediainfo.so soundkonvertercore )
target_link_libraries(soundkonverter ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} /usr/lib/libtag.so /usr/lib/libcdda_paranoia.so soundkonvertercore)

install(TARGETS soundkonverter DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

#install( FILES soundkonverter.desktop  DESTINATION  ${XDG_APPS_DIR} )
#install( FILES soundkonverter.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install(FILES soundkonverterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/soundkonverter)
install(FILES images/nocover.png  DESTINATION  ${DATA_INSTALL_DIR}/soundkonverter/images)


# dependencies: taglib, cdparanoia
### dependencies: taglib, libmediainfo, zenlib, cdparanoia

macro_display_feature_log()
