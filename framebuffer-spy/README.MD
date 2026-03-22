# framebuffer-spy

Framebuffer-spy is a simple module which provides info on the rM tablet's framebuffer.

Functions:

- FramebufferConfig getFramebufferConfig() - returns a config object, as defined in `framebuffer-spy.h`
- char *getConfigString() - returns a heap-allocated string-serialized FramebufferConfig object (`"0xAddress,width,height,type,bpl,requiresReload"`)
- void refreshFramebuffer() - this function needs to be called before accessing the framebuffer data, if the `config.requiresReload` value is set

Additionally, getConfigString is a xovi-message-broker export (`framebuffer-spy$getConfigString`), which makes it simple to access this data from software outside of xochitl.
