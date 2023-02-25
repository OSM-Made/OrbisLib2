using SQLite;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace OrbisLib2.Common.Database.App
{
    [Table("tbl_version")]
    public class AppBrowseVersion
    {
        [PrimaryKey, NotNull]
        public string category { get; set; } = "";

        public int status { get; set; }

        public static int GetAppBrowseVersion(string DataBasePath)
        {
            var db = new SQLiteConnection(DataBasePath);

            var result = db.Find((Expression<Func<AppBrowseVersion, bool>>)(x => x.category.Equals("sync_server")));
            db.Close();
            if (result != null)
            {
                return result.status;
            }
            else
            {
                return 0;
            }
        }
    }
}
