import Loki from 'lokijs';
import LokiIndexedAdapter from 'lokijs/src/loki-indexed-adapter';
const idbAdapter = new LokiIndexedAdapter();
let db;

function populate_default(){
    if ( !db.ui.findOne({}) ) {
        db.ui.insert({
            sort_type: 1,
            sort_asc: 'asc',
            audio_loop_type: '1'
        });
    }
    if ( !db.svr.findOne({}) ) {
        db.svr.insert({
            http_port: 57000,
            socks_port: 57100,
            sig_svrs: ['139.224.228.83:57000']
        });
    }
}
//export promise?
export default new Promise((resolve, reject) => {
    if (db) {
        resolve(db);
    } else {
        let mgrDB = new Loki("pyStore.db", {
            adapter: idbAdapter,
            autoload: true,
            autoloadCallback: () => {
                db = {
                    user: mgrDB.getCollection("user") ? mgrDB.getCollection("user") : mgrDB.addCollection("user"),
                    ui: mgrDB.getCollection("ui") ? mgrDB.getCollection("ui") : mgrDB.addCollection("ui"),
                    svr: mgrDB.getCollection("svr") ? mgrDB.getCollection("svr") : mgrDB.addCollection("svr")
                }
                populate_default();
                resolve(db);
            },
            autosave: true,
            autosaveInterval: 1000
        });
    }
})