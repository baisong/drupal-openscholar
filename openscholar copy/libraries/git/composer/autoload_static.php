<?php

// autoload_static.php @generated by Composer

namespace Composer\Autoload;

class ComposerStaticInitb27cb9763c30357abc27ca58a0f8f308
{
    public static $prefixLengthsPsr4 = array (
        'S' => 
        array (
            'Symfony\\Component\\Process\\' => 26,
            'Symfony\\Component\\EventDispatcher\\' => 34,
        ),
    );

    public static $prefixDirsPsr4 = array (
        'Symfony\\Component\\Process\\' => 
        array (
            0 => __DIR__ . '/..' . '/symfony/process',
        ),
        'Symfony\\Component\\EventDispatcher\\' => 
        array (
            0 => __DIR__ . '/..' . '/symfony/event-dispatcher',
        ),
    );

    public static $prefixesPsr0 = array (
        'G' => 
        array (
            'GitWrapper' => 
            array (
                0 => __DIR__ . '/..' . '/cpliakas/git-wrapper/src',
            ),
        ),
    );

    public static $classMap = array (
        'Composer\\InstalledVersions' => __DIR__ . '/..' . '/composer/InstalledVersions.php',
    );

    public static function getInitializer(ClassLoader $loader)
    {
        return \Closure::bind(function () use ($loader) {
            $loader->prefixLengthsPsr4 = ComposerStaticInitb27cb9763c30357abc27ca58a0f8f308::$prefixLengthsPsr4;
            $loader->prefixDirsPsr4 = ComposerStaticInitb27cb9763c30357abc27ca58a0f8f308::$prefixDirsPsr4;
            $loader->prefixesPsr0 = ComposerStaticInitb27cb9763c30357abc27ca58a0f8f308::$prefixesPsr0;
            $loader->classMap = ComposerStaticInitb27cb9763c30357abc27ca58a0f8f308::$classMap;

        }, null, ClassLoader::class);
    }
}
