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
            string BasePath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + @"\DarkSoulsIII";

            if (!Directory.Exists(BasePath))
            {
                MessageBox.Show("Failed to find existing save folder. Have you actually run the game before on retail?");
                return;
            }

            if (MessageBox.Show("This will overwrite any existing DS3OS saves that exist, are you sure you wish to do this?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Exclamation) != DialogResult.Yes)
            {
                return;
            }

            int FilesCopied = 0;

            string[] RetailFiles = System.IO.Directory.GetFiles(BasePath, "*.sl2", SearchOption.AllDirectories);
            foreach (string file in RetailFiles)
            {
                string NewPath = Path.ChangeExtension(file, ".ds3os");
                Console.WriteLine(file + " -> " + NewPath);

                File.Copy(file, NewPath, true);

                FilesCopied++;
            }
            
            MessageBox.Show("Copied " + FilesCopied.ToString() + " retail saves to ds3os.");
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
