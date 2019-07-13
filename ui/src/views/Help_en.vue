<template>
  <div class="help_en">
    <!-- only support android5.0 and above -->
    <h2>FAQ</h2>
    <h3>1、Where did the uploaded files go？</h3>
    <p>
    All uploaded files saved to sd card's mystore folder. app will not touch any other files or folders,
    you can use other app open uploaded files， like: use vlc open video or use microsoft word open .doc file ...
    </p>
    <h3>2、When select image, audio or video sub type, will it show this type files in all subdirectory？</h3>
    <p>
    No, only show selected type files in "current dir". 
    "current dir" shown on above header in file management page，click<i class="yellow material-icons">folder</i>enter into subdirectory，
    click<i class="green material-icons">keyboard_backspace</i>return to parent dir,
    click top-right<i class="yellow-new material-icons">create_new_folder</i>create new folder in current dir.<br>
    <span>tip: </span>when select sub type, it only show this type files, will not show sub folders.
    so if want to enter another folder, must select "all" first and then click folder that you want to enter.
    </p>    
    <h3>3、Can other client delete my files？</h3>
    <p>
    No, file deletion, movement and rename can only be done on server side. 
    client side that use browser can browse, download, paly audio or video， and upload files.
    </p>
    <h3>4、File upload and download use my mobile data traffic？</h3>
    <p>
    It use intranet data traffic, because it must connect to wifi to act as intranet web server.
    there is no public server at present， nor it send anything to public. Actually no need SIM card， only wifi it will be functional.
    </p>
    <h3>5、Does uploaded file size or number has limit？</h3>
    <p>
    No, only constrained by phone storage capacity and sd card writing speed.
    </p>
    <h3>6、Is it determine file type by file suffix？</h3>
    <p>
    No, it using magic tests.
    </p>
    <h3>7、If not connecting to charger, service still off after a while even if already added app to whitelist？</h3>
    <p>
    Some so-called deep customized android os，when app goes to background or lock screen， these os will disconnect network, 
    in this situation, must plug in charger to use phone as a server. 
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

</style>