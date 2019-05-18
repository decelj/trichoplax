#include <iostream>
#include <locale>
#include <string>
#include <limits>
#include <memory>

#include "FreeImage.h"
#include "scene.h"
#include "parser_factory.h"
#include "iparser.h"
#include "timer.h"
#include "cl_args.h"

struct Args
{
    Args();

    std::string sceneFile;
    std::string outputImage;
    std::string envSphere;

    uint32_t width;
    uint32_t height;
    uint32_t maxThreads;

    Scene::RenderSettings renderSettings;
};

Args::Args()
    : sceneFile()
    , outputImage()
    , envSphere()
    , width(0)
    , height(0)
    , maxThreads(std::numeric_limits<uint32_t>::max())
    , renderSettings()
{
}

Args parseArgs(int argc, char** argv)
{
    CLArgs argParser;
    Args args;

    argParser.RegisterArg("-o", &args.outputImage, args.outputImage);
    argParser.RegisterArg("-env", &args.envSphere, args.envSphere);
    argParser.RegisterArg("-width", &args.width, args.width);
    argParser.RegisterArg("-height", &args.height, args.height);
    argParser.RegisterArg("-maxDepth", &args.renderSettings.maxDepth, args.renderSettings.maxDepth);
    argParser.RegisterArg("-giSamples", &args.renderSettings.GISamples, args.renderSettings.GISamples);
    argParser.RegisterArg("-bias", &args.renderSettings.bias, args.renderSettings.bias);
    argParser.RegisterArg("-maxThreads", &args.maxThreads, args.maxThreads);
    
    std::vector<std::string> extraArgs;
    try
    {
        if (!argParser.Parse(argv, argc, &extraArgs))
        {
            exit(-1);
        }
        
        if (extraArgs.size() > 1)
        {
            throw std::invalid_argument(extraArgs[1]);
        }
    }
    catch (...)
    {
        argParser.PrintUsage();
        throw;
    }
    
    args.sceneFile = extraArgs[0];

    return args;
}

void applyCLOverrides(const Args& args, Scene& scene)
{
    scene.setBias(args.renderSettings.bias);
    scene.setNumGISamples(args.renderSettings.GISamples);
    scene.setMaxDepth(args.renderSettings.maxDepth);

    if (!args.envSphere.empty())
    {
        scene.setEnvSphereImage(args.envSphere);
    }

    if (args.width != 0 && args.height != 0)
    {
        scene.setImageSize(args.width, args.height);
    }
}


int main(int argc, char** argv)
{
    std::locale::global(std::locale("en_US.UTF-8"));
    std::cout.imbue(std::locale());

    if (argc <  2)
    {
        std::cerr << "Must provide an input scene file!" << std::endl;
        return -1;
    }

    Args clArgs = parseArgs(argc, argv);

    FreeImage_Initialise();
    Scene::create();
    
    {
        HighResTimer loadTimer;
        loadTimer.start();
        
        std::unique_ptr<IParser> parser = ParserFactory().create(clArgs.sceneFile);

        std::string outputImage;
        try
        {
            outputImage = parser->parse(clArgs.sceneFile, Scene::instance());
        }
        catch (...)
        {
            Scene::destroy();
            throw;
        }

        if (clArgs.outputImage.empty())
        {
            clArgs.outputImage = outputImage;
        }
        
        std::cout << "Scene load time: "
            << loadTimer.elapsedToString(loadTimer.elapsed()) << std::endl;
    }

    applyCLOverrides(clArgs, Scene::instance());

    Scene::instance().prepareForRendering();
    Scene::instance().render(clArgs.outputImage, clArgs.maxThreads);
    Scene::destroy();
    
    FreeImage_DeInitialise();
	
    return 0;
}
