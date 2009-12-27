#include "soundkonverter.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KLocale>

static const char description[] =
    I18N_NOOP("soundKonverter is a frontend to various audio encoders and decoders.");

static const char version[] = "1.0.0";

int main(int argc, char **argv)
{
    KAboutData about("soundkonverter", 0, ki18n("soundKonverter"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2007 Daniel Faust"), KLocalizedString(), 0, "hessijames@gmail.com");
    about.addAuthor( ki18n("Daniel Faust"), KLocalizedString(), "hessijames@gmail.com" );
    about.addCredit( ki18n("David Vignoni"), ki18n("Nuvola icon theme"), 0, "http://www.icon-king.com" );
    about.addCredit( ki18n("Scott Wheeler"), ki18n("TagLib"), "wheeler@kde.org", "http://ktown.kde.org/~wheeler" );
    about.addCredit( ki18n("Amarok developers"), ki18n("Amarok"), 0, "http://amarok.kde.org" );
    about.addCredit( ki18n("Kaffeine developers"), ki18n("Kaffeine"), 0, "http://kaffeine.sourceforge.net" );
    about.addCredit( ki18n("All programmers of audio converters"), ki18n("Backends") );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add( "replaygain", ki18n("Open the Replay Gain tool an add all given files") );
//     options.add( "repair", ki18n("Open the repair files tool an add all given files") );
    options.add( "rip <device>", ki18n("List all tracks on the cd drive <device>, 'auto' will search for a cd") );
    options.add( "profile <profile>", ki18n("Add all files using the given profile") );
    options.add( "format <format>", ki18n("Add all files using the given format") );
    options.add( "output <directory>", ki18n("Output all files to <directory>") );
    options.add( "invisible", ki18n("Start soundKonverter invisible") );
    options.add( "autoclose", ki18n("Close soundKonverter after all files are converted (enabled when using '--invisible')") );
    options.add( "command <command>", ki18n("Execute <command> after each file has been converted") );
    options.add( "+[files]", ki18n("Audio file(s) to append to the file list") );
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    soundKonverter *widget = new soundKonverter;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(soundKonverter);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            //soundkonverter *widget = new soundkonverter;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++)
            {
                //soundkonverter *widget = new soundkonverter;
                widget->show();
            }
        }
        args->clear();
    }

    return app.exec();
}
