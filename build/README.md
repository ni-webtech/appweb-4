
Build Directory Contents
========================

The Embedthis build system is similar to GNU autoconf system. To enhance portability to non-unix systems, 
pure autoconf is not used.

Key Files and Directories
-------------------------

* 3rd              - Files to integrate with 3rd party products
* config           - Directory for per O/S configure settings.
* configure.*      - Configure program extensions for this product
* env.config       - Build tool configuration and search paths.
* make             - Directory for per O/S make configuration.
* make.config      - Local makefile overrides. Not used.
* product.config   - Primary product configuration file (Edit this).
* *.defaults       - Configuration defaults for standard builds. See
                     standard.defaults for full comments.
