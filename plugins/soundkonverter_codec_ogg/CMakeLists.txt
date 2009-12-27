project(soundkonverter_codec_ogg)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )

set(soundkonverter_codec_ogg_SRCS
   soundkonverter_codec_ogg.cpp
   oggcodecwidget.cpp
 )

kde4_add_plugin(soundkonverter_codec_ogg WITH_PREFIX ${soundkonverter_codec_ogg_SRCS})

# kde4_add_library(soundkonverter_codec_ogg SHARED ${soundkonverter_codec_ogg_SRCS})

target_link_libraries(soundkonverter_codec_ogg ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} soundkonvertercore )
#target_link_libraries(soundkonverter_codec_ogg ${KDE4_KIO_LIBS} )

########### install files ###############

install(TARGETS soundkonverter_codec_ogg DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES soundkonverter_codec_ogg.desktop DESTINATION ${SERVICES_INSTALL_DIR})
