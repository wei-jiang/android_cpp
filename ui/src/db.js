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
                    ui: mgrDB.getCollection("ui") ? mgrDB.getCollection("ui") : mgrDB.addCollection("ui")
                }
                populate_default();
                resolve(db);
            },
            autosave: true,
            autosaveInterval: 1000
        });
    }
})