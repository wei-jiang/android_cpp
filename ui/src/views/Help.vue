<template>
  <div class="help">
    <!-- 只支持安卓5.0以上版本 -->
    <!-- <h2>操作流程</h2> -->
    <h3>1、所有的文件上传、操作都限制在安卓sd卡的mystore目录</h3>
    <p>
    应用不会触及其它目录和文件，上传的文件可用其它应用打开，比如用其它播放器播放其中音频/视频，用微软word打开.doc文件……
    </p>
    <h3>2、只显示/操作【当前目录】中的文件夹和文件，并不会显示子目录中的内容</h3>
    <p>
    【当前目录】显示在“文件管理”上面的标题栏，点<i class="material-icons">folder</i>进入子目录，点<i class="material-icons">keyboard_backspace</i>返回上级目录。
    点右上角的<i class="material-icons">create_new_folder</i>在当前目录中新建子目录。<br>
    <span>注意：</span>移动文件时，选择目标文件夹点确定后，【当前目录】会返回到选择前的目录。
    </p>    
    <h3>3、文件的删除、移动、重命名等操作只在手机服务端开放</h3>
    <p>
    因为这个应用相当于一个文件服务器，客户端只能浏览、下载、播放（用浏览器）音频/视频，和上传文件，在该应用中（服务端）才可删除、移动文件。
    </p>
    <h3>4、文件的上传、下载都是用内网流量</h3>
    <p>
    不会用到公网流量，目前没有公网服务器，也不会发送任何东西到公网上去。实际上只开4g没连wifi，根本就访问不了手机上的文件。
    </p>
    <h3>5、上传的文件大小没限制</h3>
    <p>
    只与手机的存储容量、sd卡读写速度，和内网网速有关。有的手机sd卡写入文件会比较慢。
    </p>
    <h3>6、不会根据后缀名判断文件类型</h3>
    <p>
    比如把[a.jpg]改名为[a]还是图片文件
    </p>
  </div>
</template>

<style scoped>
.faq{
  margin-top: 1em;
}
p, h3{
  text-align: left;
  margin: 1.0em 1.0em;
}
span {
  color: red;
}
.to-apply{
  margin: 1.9em 1em;
  font-size: 1.7rem;
}
</style>