namespace gui
{
    partial class Form1
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
            this.rbLKmethod = new System.Windows.Forms.RadioButton();
            this.rbFBmethod = new System.Windows.Forms.RadioButton();
            this.rbDMmethod = new System.Windows.Forms.RadioButton();
            this.rbMOG2 = new System.Windows.Forms.RadioButton();
            this.rbMOG = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbPath = new System.Windows.Forms.TextBox();
            this.btFileSelect = new System.Windows.Forms.Button();
            this.engineInitializer = new System.ComponentModel.BackgroundWorker();
            this.btCompute = new System.Windows.Forms.Button();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.engineInformer = new System.ComponentModel.BackgroundWorker();
            this.label2 = new System.Windows.Forms.Label();
            this.lbStatus = new System.Windows.Forms.Label();
            this.engineResponce = new System.ComponentModel.BackgroundWorker();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // rbLKmethod
            // 
            this.rbLKmethod.AutoSize = true;
            this.rbLKmethod.Checked = true;
            this.rbLKmethod.Location = new System.Drawing.Point(6, 18);
            this.rbLKmethod.Name = "rbLKmethod";
            this.rbLKmethod.Size = new System.Drawing.Size(94, 17);
            this.rbLKmethod.TabIndex = 0;
            this.rbLKmethod.TabStop = true;
            this.rbLKmethod.Text = "Lucas-Kanade";
            this.rbLKmethod.UseVisualStyleBackColor = true;
            // 
            // rbFBmethod
            // 
            this.rbFBmethod.AutoSize = true;
            this.rbFBmethod.Location = new System.Drawing.Point(6, 41);
            this.rbFBmethod.Name = "rbFBmethod";
            this.rbFBmethod.Size = new System.Drawing.Size(76, 17);
            this.rbFBmethod.TabIndex = 1;
            this.rbFBmethod.Text = "Farneback";
            this.rbFBmethod.UseVisualStyleBackColor = true;
            // 
            // rbDMmethod
            // 
            this.rbDMmethod.AutoSize = true;
            this.rbDMmethod.Location = new System.Drawing.Point(6, 64);
            this.rbDMmethod.Name = "rbDMmethod";
            this.rbDMmethod.Size = new System.Drawing.Size(140, 17);
            this.rbDMmethod.TabIndex = 2;
            this.rbDMmethod.Text = "Dense motion estimation";
            this.rbDMmethod.UseVisualStyleBackColor = true;
            // 
            // rbMOG2
            // 
            this.rbMOG2.AutoSize = true;
            this.rbMOG2.Location = new System.Drawing.Point(6, 43);
            this.rbMOG2.Name = "rbMOG2";
            this.rbMOG2.Size = new System.Drawing.Size(142, 17);
            this.rbMOG2.TabIndex = 3;
            this.rbMOG2.Text = "MOG2 (high accept rate)";
            this.rbMOG2.UseVisualStyleBackColor = true;
            // 
            // rbMOG
            // 
            this.rbMOG.AutoSize = true;
            this.rbMOG.Checked = true;
            this.rbMOG.Location = new System.Drawing.Point(6, 20);
            this.rbMOG.Name = "rbMOG";
            this.rbMOG.Size = new System.Drawing.Size(132, 17);
            this.rbMOG.TabIndex = 4;
            this.rbMOG.TabStop = true;
            this.rbMOG.Text = "MOG (low accept rate)";
            this.rbMOG.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.rbFBmethod);
            this.groupBox1.Controls.Add(this.rbLKmethod);
            this.groupBox1.Controls.Add(this.rbDMmethod);
            this.groupBox1.Location = new System.Drawing.Point(3, 19);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(153, 87);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Flow calculation method";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.rbMOG2);
            this.groupBox2.Controls.Add(this.rbMOG);
            this.groupBox2.Location = new System.Drawing.Point(162, 19);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(176, 87);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Moving object detection method";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "File name";
            // 
            // tbPath
            // 
            this.tbPath.Location = new System.Drawing.Point(68, 6);
            this.tbPath.Name = "tbPath";
            this.tbPath.ReadOnly = true;
            this.tbPath.Size = new System.Drawing.Size(204, 20);
            this.tbPath.TabIndex = 8;
            // 
            // btFileSelect
            // 
            this.btFileSelect.Location = new System.Drawing.Point(278, 4);
            this.btFileSelect.Name = "btFileSelect";
            this.btFileSelect.Size = new System.Drawing.Size(75, 23);
            this.btFileSelect.TabIndex = 9;
            this.btFileSelect.Text = "Select file";
            this.btFileSelect.UseVisualStyleBackColor = true;
            this.btFileSelect.Click += new System.EventHandler(this.btFileSelect_Click);
            // 
            // engineInitializer
            // 
            this.engineInitializer.DoWork += new System.ComponentModel.DoWorkEventHandler(this.engineInitializer_DoWork);
            // 
            // btCompute
            // 
            this.btCompute.Enabled = false;
            this.btCompute.Location = new System.Drawing.Point(278, 148);
            this.btCompute.Name = "btCompute";
            this.btCompute.Size = new System.Drawing.Size(75, 23);
            this.btCompute.TabIndex = 10;
            this.btCompute.Text = "Compute";
            this.btCompute.UseVisualStyleBackColor = true;
            this.btCompute.Click += new System.EventHandler(this.btCompute_Click);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.groupBox2);
            this.groupBox3.Controls.Add(this.groupBox1);
            this.groupBox3.Location = new System.Drawing.Point(12, 33);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(341, 109);
            this.groupBox3.TabIndex = 11;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Options";
            // 
            // engineInformer
            // 
            this.engineInformer.DoWork += new System.ComponentModel.DoWorkEventHandler(this.engineInformer_DoWork);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 153);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(40, 13);
            this.label2.TabIndex = 12;
            this.label2.Text = "Status:";
            // 
            // lbStatus
            // 
            this.lbStatus.AutoSize = true;
            this.lbStatus.Location = new System.Drawing.Point(53, 153);
            this.lbStatus.Name = "lbStatus";
            this.lbStatus.Size = new System.Drawing.Size(35, 13);
            this.lbStatus.TabIndex = 13;
            this.lbStatus.Text = "label3";
            // 
            // engineResponce
            // 
            this.engineResponce.DoWork += new System.ComponentModel.DoWorkEventHandler(this.engineResponce_DoWork);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(369, 187);
            this.Controls.Add(this.lbStatus);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.btCompute);
            this.Controls.Add(this.btFileSelect);
            this.Controls.Add(this.tbPath);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Smoke detector";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RadioButton rbLKmethod;
        private System.Windows.Forms.RadioButton rbFBmethod;
        private System.Windows.Forms.RadioButton rbDMmethod;
        private System.Windows.Forms.RadioButton rbMOG2;
        private System.Windows.Forms.RadioButton rbMOG;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbPath;
        private System.Windows.Forms.Button btFileSelect;
        private System.ComponentModel.BackgroundWorker engineInitializer;
        private System.Windows.Forms.Button btCompute;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.ComponentModel.BackgroundWorker engineInformer;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label lbStatus;
        private System.ComponentModel.BackgroundWorker engineResponce;
    }
}

