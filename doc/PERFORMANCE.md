```
134400 gfxBuffer.getPixel calls took 66371us, 0.493839us/call
134400 epepd.getBwRam()->getPixel calls took 67479us, 0.502113us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3769us, 0.224583us/call
16800 epepd.getBwRam()->_streamInBytes calls took 4118us, 0.245357us/call
134400 gfxBuffer.drawPixel calls took 102502us, 0.762701us/call
```

```
134400 gfxBuffer.getPixel calls took 66375us, 0.493869us/call
134400 epepd.getBwRam()->getPixel calls took 67475us, 0.502083us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3205us, 0.191012us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3193us, 0.190357us/call
134400 gfxBuffer.drawPixel calls took 102498us, 0.762664us/call
```

```
134400 gfxBuffer.getPixel calls took 75967us, 0.565260us/call
134400 epepd.getBwRam()->getPixel calls took 77058us, 0.573385us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3197us, 0.190595us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3200us, 0.190536us/call
134400 gfxBuffer.drawPixel calls took 93460us, 0.695394us/call
134400 GFXcanvas1.getPixel calls took 54493us, 0.405491us/call
134400 GFXcanvas1.drawPixel calls took 107950us, 0.803229us/call
134400 GFXcanvas8.getPixel calls took 41016us, 0.305216us/call
134400 GFXcanvas8.drawPixel calls took 70207us, 0.522381us/call
```

Bitmap slow

```
134400 gfxBuffer.getPixel calls took 100809us, 0.750097us/call
134400 epepd.getBwRam()->getPixel calls took 101881us, 0.758073us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3053us, 0.182024us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3057us, 0.182024us/call
134400 gfxBuffer.drawPixel calls took 108672us, 0.808579us/call
134400 GFXcanvas1.getPixel calls took 54497us, 0.405521us/call
134400 GFXcanvas1.drawPixel calls took 107954us, 0.803237us/call
134400 GFXcanvas8.getPixel calls took 41012us, 0.305186us/call
134400 GFXcanvas8.drawPixel calls took 70187us, 0.522254us/call
```

After some optimization

```
134400 gfxBuffer.getPixel calls took 83305us, 0.619859us/call
134400 epepd.getBwRam()->getPixel calls took 84388us, 0.627894us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3058us, 0.182024us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3057us, 0.182024us/call
134400 gfxBuffer.drawPixel calls took 108709us, 0.808877us/call
134400 GFXcanvas1.getPixel calls took 54507us, 0.405565us/call
134400 GFXcanvas1.drawPixel calls took 107958us, 0.803259us/call
134400 GFXcanvas8.getPixel calls took 41013us, 0.305186us/call
134400 GFXcanvas8.drawPixel calls took 70206us, 0.522403us/call
```

before setPixel optimization

```
134400 gfxBuffer.getPixel calls took 77658us, 0.577842us/call
134400 epepd.getBwRam()->getPixel calls took 78739us, 0.585863us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3065us, 0.182440us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3058us, 0.182083us/call
134400 gfxBuffer.drawPixel calls took 108672us, 0.808601us/call
134400 GFXcanvas1.getPixel calls took 54504us, 0.405543us/call
134400 GFXcanvas1.drawPixel calls took 107958us, 0.803259us/call
134400 GFXcanvas8.getPixel calls took 41013us, 0.305193us/call
134400 GFXcanvas8.drawPixel calls took 70191us, 0.522292us/call
```

and after... (meh)

```
134400 gfxBuffer.getPixel calls took 77668us, 0.577917us/call
134400 epepd.getBwRam()->getPixel calls took 78746us, 0.585915us/call
16800 epepd.getBwRam()->_streamOutBytes calls took 3061us, 0.182262us/call
16800 epepd.getBwRam()->_streamInBytes calls took 3057us, 0.182024us/call
134400 gfxBuffer.drawPixel calls took 98549us, 0.733281us/call
134400 GFXcanvas1.getPixel calls took 54512us, 0.405603us/call
134400 GFXcanvas1.drawPixel calls took 107966us, 0.803318us/call
134400 GFXcanvas8.getPixel calls took 41009us, 0.305164us/call
134400 GFXcanvas8.drawPixel calls took 70193us, 0.522299us/call
```

156321us
