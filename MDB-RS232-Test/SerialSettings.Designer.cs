namespace MDB_RS232_Test
{
    partial class SerialSettingsForm
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
            this.SerialSelect = new System.Windows.Forms.ComboBox();
            this.OKButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // SerialSelect
            // 
            this.SerialSelect.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.SerialSelect.FormattingEnabled = true;
            this.SerialSelect.Location = new System.Drawing.Point(14, 12);
            this.SerialSelect.Name = "SerialSelect";
            this.SerialSelect.Size = new System.Drawing.Size(140, 21);
            this.SerialSelect.TabIndex = 0;
            this.SerialSelect.SelectedIndexChanged += new System.EventHandler(this.SerialSelect_SelectedIndexChanged);
            // 
            // OKButton
            // 
            this.OKButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.OKButton.Enabled = false;
            this.OKButton.Location = new System.Drawing.Point(14, 39);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(140, 23);
            this.OKButton.TabIndex = 1;
            this.OKButton.Text = "OK";
            this.OKButton.UseVisualStyleBackColor = true;
            this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
            // 
            // SerialSettingsForm
            // 
            this.AcceptButton = this.OKButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(167, 75);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.SerialSelect);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SerialSettingsForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Select COM";
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button OKButton;
        public System.Windows.Forms.ComboBox SerialSelect;
    }
}