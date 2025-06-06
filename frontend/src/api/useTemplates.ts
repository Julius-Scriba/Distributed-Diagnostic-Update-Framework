import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';
import api from '../api';

export interface TemplateEntry {
  template_id: string;
  name: string;
  command: string;
  parameters: Record<string, unknown>;
}

interface TemplatesResponse {
  templates: TemplateEntry[];
}

export function useTemplates() {
  return useQuery({
    queryKey: ['templates'],
    queryFn: async () => {
      const { data } = await api.get<TemplatesResponse>('/admin/templates');
      return data.templates;
    },
  });
}

interface TemplatePayload {
  name: string;
  command: string;
  parameters: Record<string, unknown>;
}

export function useCreateTemplate() {
  const qc = useQueryClient();
  return useMutation({
    mutationFn: async (payload: TemplatePayload) => {
      await api.post('/admin/templates', payload);
    },
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['templates'] });
    },
  });
}

export function useDeleteTemplate() {
  const qc = useQueryClient();
  return useMutation({
    mutationFn: async (id: string) => {
      await api.delete(`/admin/templates/${id}`);
    },
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['templates'] });
    },
  });
}
