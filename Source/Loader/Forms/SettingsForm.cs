using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Loader
{
    public partial class SettingsForm : Form
    {
        public string ExeLocation = "";
        private bool DoNotSaveSettings = false;

        public SettingsForm()
        {
            InitializeComponent();
        }

        private void OnLoad(object sender, EventArgs e)
        {
            DoNotSaveSettings = true;
            UseSeperateSavesCheckbox.Checked = ProgramSettings.Default.use_seperate_saves;
            DoNotSaveSettings = false;

            UpdateState();
        }

        private void UpdateState()
        {
            CopySavesButton.Enabled = ProgramSettings.Default.use_seperate_saves;
        }

        private void CopySavesClicked(object sender, EventArgs e)
        {   
            if (MessageBox.Show("This will overwrite any existing DSOS saves that exist, are you sure you wish to do this?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) != DialogResult.Yes)
            {
                return;
            }

            int FilesCopied = 0;
            
            string BasePath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\DarkSoulsIII";
            FilesCopied += CopySavesInDirectory(BasePath);
            
            BasePath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\DarkSoulsII";
            FilesCopied += CopySavesInDirectory(BasePath);
            
            MessageBox.Show("Copied " + FilesCopied.ToString() + " retail saves to dsos.");
        }

        private int CopySavesInDirectory(string BasePath)
        {
            int FilesCopied = 0;
            
            if (!Directory.Exists(BasePath))
            {
                return 0;
            }

            string[] RetailFiles = System.IO.Directory.GetFiles(BasePath, "*.sl2", SearchOption.AllDirectories);
            foreach (string file in RetailFiles)
            {
                string NewPath = Path.ChangeExtension(file, ".ds3os");
                Console.WriteLine(file + " -> " + NewPath);

                File.Copy(file, NewPath, true);

                FilesCopied++;
            }
            
            return FilesCopied;
        }

        private void SettingChanged(object sender, EventArgs e)
        {
            if (DoNotSaveSettings)
            {
                return;
            }

            ProgramSettings.Default.use_seperate_saves = UseSeperateSavesCheckbox.Checked;
            ProgramSettings.Default.Save();

            UpdateState();
        }
    }
}
