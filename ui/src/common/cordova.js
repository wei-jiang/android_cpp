
class Cordova{
    // cordova.file.externalRootDirectory
    create_dir_recursive(path){
        const dirs = path.split("/");
        // console.log(JSON.stringify(dirs))
        return new Promise((resolve, reject) => {
            window.resolveLocalFileSystemURL(cordova.file.externalRootDirectory, (dirEntry)=> {
                // console.log(JSON.stringify(dirEntry))
                function create_sub_dir(parent, subs){
                    // console.log(`parent=${JSON.stringify(parent)}; subs=${JSON.stringify(subs)}`)
                    const sub = subs.shift();
                    if(sub){
                        parent.getDirectory(sub, { create: true }, subDirEntry=>{
                            create_sub_dir(subDirEntry, subs)
                        }, err=>reject(err));
                    } else{
                        resolve(parent)
                    }                
                }
                create_sub_dir(dirEntry, dirs);
            }, err=>reject(err));
        });        
    }
    get_fileEntry(path){
        return new Promise((resolve, reject) => {
            window.resolveLocalFileSystemURL(path, (fileEntry)=> {
                resolve(fileEntry);
            }, err=>reject(err));
        }); 
    }
}
// /storage/emulated/0/mystore/inout/in/image/1394baf0-c9a2-11e9-bea8-7529131a2786.jpg
// file:///storage/emulated/0/mystore/inout/in/img/8bce8d50-c896-11e9-b0d5-0185c0b36f10.mp3
export default Cordova;