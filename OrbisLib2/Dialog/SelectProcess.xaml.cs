using OrbisLib2.Targets;
using SimpleUI.Controls;
using SimpleUI.Dialogs;
using System.Windows;
using System.Windows.Controls.Primitives;

namespace OrbisLib2.Dialog
{
    /// <summary>
    /// Interaction logic for SelectProcess.xaml
    /// </summary>
    public partial class SelectProcess : SimpleDialog
    {
        public SelectProcess(Window Owner)
            : base(Owner, "Select", "Cancel", "Select Process")
        {
            InitializeComponent();

            // Get initial process list.
            RefreshProcessList();
        }

        public static SimpleDialogResult ShowDialog(Window Owner)
        {
            var dlg = new SelectProcess(Owner);
            dlg.ShowDialog();

            var selectedProc = (ProcInfo)dlg.ProcessList.SelectedItem;
            if(selectedProc != null)
            {
                TargetManager.SelectedTarget.Debug.Attach(selectedProc.ProcessId);
            }

            return dlg.Result;
        }

        private void RefreshProcessList()
        {
            Task.Run(() =>
            {
                var procList = new List<ProcInfo>();
                var result = TargetManager.SelectedTarget.GetProcList(out procList);

                // Print the error that occured if we failed to get the process list.
                if (!result.Succeeded)
                {
                    Dispatcher.Invoke(() =>
                    {
                        ProcessList.ItemsSource = null;
                    });

                    SimpleMessageBox.ShowError(Window.GetWindow(this), result.ErrorMessage, "Failed to refresh process list.");
                    return;
                }

                // Refresh the list.
                Dispatcher.Invoke(() =>
                {
                    ProcessList.ItemsSource = procList;
                    ProcessList.Items.Refresh();
                });
            });
        }

        private void Refresh_Click(object sender, RoutedEventArgs e)
        {
            RefreshProcessList();
        }

        private void ProcessList_MouseDoubleClick(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            var item = ((FrameworkElement)e.OriginalSource).DataContext as ProcInfo;
            if (item != null && ProcessList.SelectedItem != null)
            {
                Result = SimpleDialogResult.Button1;
                Close();
            }
        }
    }
}
