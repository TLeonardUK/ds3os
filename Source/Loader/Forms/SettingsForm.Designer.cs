namespace Loader
{
    partial class SettingsForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
        if (disposing && (components != null))
        {
        components.Dispose();
        }
        base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingsForm));
        UseSeperateSavesCheckbox = new System.Windows.Forms.CheckBox();
        label1 = new System.Windows.Forms.Label();
        CopySavesButton = new System.Windows.Forms.Button();
        SuspendLayout();
        // 
        // UseSeperateSavesCheckbox
        // 
        UseSeperateSavesCheckbox.AutoSize = true;
        UseSeperateSavesCheckbox.Location = new System.Drawing.Point(26, 27);
        UseSeperateSavesCheckbox.Name = "UseSeperateSavesCheckbox";
        UseSeperateSavesCheckbox.Size = new System.Drawing.Size(147, 19);
        UseSeperateSavesCheckbox.TabIndex = 0;
        UseSeperateSavesCheckbox.Text = "Use seperate save files?";
        UseSeperateSavesCheckbox.UseVisualStyleBackColor = true;
        UseSeperateSavesCheckbox.CheckedChanged += SettingChanged;
        // 
        // label1
        // 
        label1.Location = new System.Drawing.Point(45, 49);
        label1.Name = "label1";
        label1.Size = new System.Drawing.Size(448, 97);
        label1.TabIndex = 1;
        label1.Text = resources.GetString("label1.Text");
        // 
        // CopySavesButton
        // 
        CopySavesButton.Location = new System.Drawing.Point(320, 149);
        CopySavesButton.Name = "CopySavesButton";
        CopySavesButton.Size = new System.Drawing.Size(173, 40);
        CopySavesButton.TabIndex = 2;
        CopySavesButton.Text = "Copy Retail Saves to DSOS";
        CopySavesButton.UseVisualStyleBackColor = true;
        CopySavesButton.Click += CopySavesClicked;
        // 
        // SettingsForm
        // 
        AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
        AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
        ClientSize = new System.Drawing.Size(528, 223);
        Controls.Add(CopySavesButton);
        Controls.Add(label1);
        Controls.Add(UseSeperateSavesCheckbox);
        FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
        Name = "SettingsForm";
        ShowInTaskbar = false;
        StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
        Text = "Settings";
        Load += OnLoad;
        ResumeLayout(false);
        PerformLayout();
        }

        #endregion

        private System.Windows.Forms.CheckBox UseSeperateSavesCheckbox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button CopySavesButton;
    }
}