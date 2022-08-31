# Zygisk-ModuleTemplate
Zygisk module template. refer to  [Riru-ModuleTemplate](https://github.com/RikkaApps/Riru-ModuleTemplate) and [zygisk-module-sample](https://github.com/topjohnwu/zygisk-module-sample)

## 说明
由于riru模块已经后可取代的新zygisk,所以参考的别人的仓库生成了一个自动构建zygisk模块的模板

构建这个项目的时候遇到了很多坑，这里都修复好了的,只需要关注  ##构建 注意就行

## 构建
修改[module.gradle]里面的模块信息                                   [可选]

修改[local.properties]为自己的sdk路径,这个很重要                     [必须]

你也可以修改/module/build.gradle里面的ndk和cmake版本为自己的版本       [必须]

需要配置jdk11的环境                                                [必须]

构建方法1
```
在菜单栏里面打开构建选项(builder),点击Make Project 快捷键“[Ctrl + F9]”
```

构建方法2
```
打开命令行,运行 gradlew :module:assembleRelease
```

构建方法3
```
运行 [build.bat]脚本文件
```

如果构建成功，生成的zip模块文件在/out文件夹里. 把生成zip传到手机里就可以用magisk安装了.

可使用命令
```
adb -s target push xxxxx.zip /sdcard/
```

如果安装成功会看到如下图片
![png](/img/install.png)

如果安装的模块生效,则就可以可以通过logcat看到如下
![png](/img/show.png)

