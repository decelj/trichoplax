#include <iostream>
#include <locale>
#include <string>
#include <limits>

#include "FreeImage.h"
#include "scene.h"
#include "parser_factory.h"
#include "iparser.h"
#include "timer.h"

struct Args
{
    Args();

    std::string sceneFile;
    std::string outputImage;
    std::string envSphere;

    unsigned width;
    unsigned height;

    Scene::RenderSettings renderSettings;
};

Args::Args()
    : sceneFile()
    , outputImage()
    , envSphere()
    , width(0)
    , height(0)
    , renderSettings()
{
    renderSettings.maxDepth = std::numeric_limits<unsigned>::max();
    renderSettings.GISamples = std::numeric_limits<unsigned>::max();
    renderSettings.bias = -1.f;
}

Args parseArgs(int argc, char** argv)
{
    Args args;

    // TODO: Create a nicer CL parser :)
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected output file");
            }

            args.outputImage = argv[++i];
        }
        else if (strcmp(argv[i], "-env") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected env sphere file");
            }

            args.envSphere = argv[++i];
        }
        else if (strcmp(argv[i], "-width") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected width number");
            }

            args.width = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-height") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected height number");
            }

            args.height = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-maxDepth") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected max depth number");
            }

            args.renderSettings.maxDepth = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-giSamples") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected number of gi samples");
            }

            args.renderSettings.GISamples = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-bias") == 0)
        {
            if (i >= argc - 1)
            {
                throw std::runtime_error("expected bias number");
            }

            args.renderSettings.bias = (float)atof(argv[++i]);
        }
        else
        {
            args.sceneFile = argv[i];
        }
    }

    return args;
}

void applyCLOverrides(const Args& args, Scene& scene)
{
    if (args.renderSettings.bias != -1.f)
    {
        scene.setBias(args.renderSettings.bias);
    }

    if (args.renderSettings.GISamples != std::numeric_limits<unsigned>::max())
    {
        scene.setNumGISamples(args.renderSettings.GISamples);
    }

    if (args.renderSettings.maxDepth != std::numeric_limits<unsigned>::max())
    {
        scene.setMaxDepth(args.renderSettings.maxDepth);
    }

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
        
        IParser* parser = ParserFactory().create(clArgs.sceneFile);

        std::string outputImage;
        try
        {
            outputImage = parser->parse(clArgs.sceneFile, Scene::instance());
        }
        catch (...)
        {
            delete parser;
            Scene::destroy();
            throw;
        }
        
        delete parser;

        if (clArgs.outputImage.empty())
        {
            clArgs.outputImage = outputImage;
        }
        
        std::cout << "Scene load time: "
            << loadTimer.elapsedToString(loadTimer.elapsed()) << std::endl;
    }

    applyCLOverrides(clArgs, Scene::instance());

    Scene::instance().render(clArgs.outputImage);
    Scene::destroy();
    
    FreeImage_DeInitialise();
	
    return 0;
}
