import { useMutation } from '@tanstack/react-query';
import api from '../api';

interface CommandPayload {
  command: string;
  parameters?: Record<string, unknown>;
}

export default function useSendCommand(uuid: string) {
  return useMutation({
    mutationFn: async (payload: CommandPayload) => {
      await api.post(`/admin/command/${uuid}`, payload);
    },
  });
}
